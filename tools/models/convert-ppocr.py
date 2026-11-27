#!/usr/bin/env python3
"""The handwriting line model of the release models-5: PP-OCRv6 small (kraken's recognition model, Apache License 2.0)
converted to ONNX for the program's Handwriting (ONNX Runtime), with its codec as a JSON file beside it.

  python3 -I tools/models/convert-ppocr.py <small.safetensors> <outdir> [--height 96] [--padding 16]

Needs (the workflow installs them): torch (2.9 to 2.11, CPU), safetensors, onnx, onnxruntime, numpy, pillow, and
kraken 7.2 installed with --no-deps (only its source of the network is used: kraken's own package imports Lightning and
a dozen more libraries that the conversion does not need, so the network's files are loaded from the installed
folder without kraken's package initializer).

What it does:
  - reads the weights and the metadata kraken wrote into the file (variant, number of classes, height, codec),
  - builds the network (backbone PPLCNetV4, neck, CTC head), loads the weights strictly (no missing or unexpected key),
  - exports it with a fixed input height (the backbone's last avg_pool2d(kernel_size=(h, 2)) takes h from the tensor,
    so the trace must see a fixed h) and a free width: input (1, 3, 96, W), output logits (1, 1623, 1, T), T = W / 8,
  - checks that ONNX Runtime and PyTorch agree (largest difference under 0.001) on lines rendered here, at several
    widths, and that both decode to the same text,
  - writes <name>.onnx and <name>-codec.json and prints the parameter count of the network and the files' sizes.

The program's side of it (these are the facts the notes of the earlier measurement and kraken's input transforms give):
  input  = the line's ink image, resized to height 96 (Lanczos), 16 px white padding left and right, RGB, value =
           1 - pixel / 255 (white is 0, ink is near 1)
  output = logits (1, 1623, 1, T); class 0 is the CTC blank; greedy CTC (argmax, collapse repeats, drop blanks);
           labels map to graphemes through the codec (a grapheme may have several labels: longest match as kraken's
           PytorchCodec.decode does); the text is NFD with combining marks, so NFC after decoding. The marks U+0300 to
           U+0303, U+0308 and U+0327 must be kept by anything that filters the output.
"""
import argparse
import importlib.util
import json
import os
import struct
import sys
import types
import unicodedata

EXPECTED_CLASSES = 1623
KEEP_MARKS = [0x0300, 0x0301, 0x0302, 0x0303, 0x0308, 0x0327]
LICENSE_OF_THE_MODEL = "Apache-2.0"


def read_safetensors_metadata(path):
    with open(path, "rb") as fh:
        n = struct.unpack("<Q", fh.read(8))[0]
        header = json.loads(fh.read(n))
    return header.get("__metadata__", {})


def load_network_modules():
    """The package of kraken's PP-OCRv6 network (lib/ppocr) without kraken's own heavy initializers."""
    spec = importlib.util.find_spec("kraken")
    if spec is None or not spec.submodule_search_locations:
        sys.exit("kraken is not installed (pip install --no-deps kraken==7.2)")
    folder = os.path.join(list(spec.submodule_search_locations)[0], "lib", "ppocr")
    if not os.path.isfile(os.path.join(folder, "network.py")):
        sys.exit(f"{folder}/network.py is missing: this kraken has no PP-OCRv6")
    pkg = types.ModuleType("ppocr_net")
    pkg.__path__ = [folder]
    sys.modules["ppocr_net"] = pkg
    import ppocr_net.network as network  # noqa: E402  (a relative import inside it needs the package above)
    return network


def build_model(path):
    import torch
    from safetensors.torch import load_file

    meta = json.loads(read_safetensors_metadata(path)["kraken_meta"])
    prefix = next((k for k, v in meta.items() if v.get("_model") == "PPOCRv6Model"), None)
    if prefix is None:
        sys.exit("the file holds no PPOCRv6Model")
    info = meta[prefix]
    network = load_network_modules()
    net = network.build_recognizer(info["variant"], num_classes=info["num_classes"])
    state = load_file(path)
    head = prefix + ".nn."
    mine = {k[len(head):]: v for k, v in state.items() if k.startswith(head)}
    result = net.load_state_dict(mine, strict=False)
    if result.missing_keys or result.unexpected_keys:
        sys.exit(f"the weights do not fit the network: missing {result.missing_keys[:5]}, "
                 f"unexpected {result.unexpected_keys[:5]}")
    net.eval()
    for p in net.parameters():
        p.requires_grad_(False)
    return net, info


class Codec:
    """kraken's PytorchCodec.decode for a CTC labelling (blank 0), nothing else."""

    def __init__(self, c2l):
        self.c2l = {k: list(v) for k, v in c2l.items()}
        self.l2c = {tuple(v): k for k, v in self.c2l.items()}
        self.single = {v[0]: k for v, k in ((tuple(v), k) for k, v in self.c2l.items()) if len(v) == 1}

    def decode(self, labels):
        out, i = [], 0
        while i < len(labels):
            if labels[i] in self.single:
                out.append(self.single[labels[i]])
                i += 1
                continue
            for code, ch in self.l2c.items():
                if tuple(labels[i:i + len(code)]) == code:
                    out.append(ch)
                    i += len(code)
                    break
            else:
                i += 1
        return "".join(out)


def greedy_ctc(logits):
    """logits (1, C, 1, T) -> the label sequence: argmax, collapse repeats, drop the blank (class 0)."""
    import numpy as np

    best = np.asarray(logits)[0, :, 0, :].argmax(axis=0)
    labels, last = [], -1
    for b in best.tolist():
        if b != last and b != 0:
            labels.append(b)
        last = b
    return labels


def prepare(image, height, padding):
    """A PIL image of a line (dark ink on light) -> the network's input (1, 3, height, W) as float32."""
    import numpy as np
    from PIL import Image

    gray = image.convert("L")
    w, h = gray.size
    new_w = max(8, round(w * height / h))
    gray = gray.resize((new_w, height), Image.LANCZOS)
    canvas = Image.new("L", (new_w + 2 * padding, height), 255)
    canvas.paste(gray, (padding, 0))
    arr = np.asarray(canvas.convert("RGB"), dtype=np.float32) / 255.0
    return (1.0 - arr).transpose(2, 0, 1)[None].copy()


def render_lines():
    """A few lines of text drawn here (no data of anybody): short, long, with accents."""
    from PIL import Image, ImageDraw, ImageFont

    candidates = ["/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
                  "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf"]
    fonts = []
    for c in candidates:
        if os.path.isfile(c):
            fonts.append(ImageFont.truetype(c, 44))
    if not fonts:
        fonts = [ImageFont.load_default(size=44)]
    texts = ["Hello world", "The quick brown fox jumps over the lazy dog", "Voyez le brick geant qui j'examine",
             "Zwölf Boxkämpfer jagen Viktor quer über den großen Sylter Deich", "El veloz murciélago hindú"]
    lines = []
    for i, t in enumerate(texts):
        font = fonts[i % len(fonts)]
        box = ImageDraw.Draw(Image.new("L", (10, 10))).textbbox((0, 0), t, font=font)
        img = Image.new("L", (box[2] + 24, box[3] + 24), 255)
        ImageDraw.Draw(img).text((12, 8), t, fill=0, font=font)
        lines.append((t, img))
    return lines


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("weights")
    ap.add_argument("outdir")
    ap.add_argument("--name", default="ppocrv6-small")
    ap.add_argument("--height", type=int, default=96)
    ap.add_argument("--padding", type=int, default=16)
    ap.add_argument("--max-difference", type=float, default=0.001)
    args = ap.parse_args()

    import numpy as np
    import onnx
    import onnxruntime as ort
    import torch

    os.makedirs(args.outdir, exist_ok=True)
    net, info = build_model(args.weights)
    n_params = sum(p.numel() for p in net.parameters())
    print(f"model: variant {info['variant']}, classes {info['num_classes']}, height in the file {info['height']}, "
          f"parameters {n_params:,} ({n_params / 1e6:.2f} M), file {os.path.getsize(args.weights):,} bytes")
    if info["num_classes"] != EXPECTED_CLASSES:
        sys.exit(f"expected {EXPECTED_CLASSES} classes, the file has {info['num_classes']}")
    if info["height"] != args.height:
        print(f"::warning::the file says line height {info['height']}, the export uses {args.height}")

    class Wrapped(torch.nn.Module):
        def __init__(self, inner):
            super().__init__()
            self.inner = inner

        def forward(self, x):
            return self.inner(x, None)[0]

    wrapped = Wrapped(net).eval()
    dummy = torch.rand(1, 3, args.height, 256)
    onnx_path = os.path.join(args.outdir, args.name + ".onnx")
    kwargs = dict(input_names=["input"], output_names=["logits"],
                  dynamic_axes={"input": {3: "width"}, "logits": {3: "steps"}}, opset_version=17)
    with torch.no_grad():
        try:
            torch.onnx.export(wrapped, (dummy,), onnx_path, dynamo=False, **kwargs)
        except TypeError:   # torch before 2.5 has no dynamo switch (and always traces)
            torch.onnx.export(wrapped, (dummy,), onnx_path, **kwargs)
    onnx.checker.check_model(onnx.load(onnx_path))

    codec = Codec(info["codec"])
    sess = ort.InferenceSession(onnx_path, providers=["CPUExecutionProvider"])
    worst = 0.0
    shown = []
    for text, img in render_lines():
        x = prepare(img, args.height, args.padding)
        with torch.no_grad():
            ref = wrapped(torch.from_numpy(x)).numpy()
        got = sess.run(None, {"input": x})[0]
        if got.shape != ref.shape:
            sys.exit(f"shape differs for '{text}': onnx {got.shape}, torch {ref.shape}")
        diff = float(np.abs(got - ref).max())
        worst = max(worst, diff)
        t_ref = unicodedata.normalize("NFC", codec.decode(greedy_ctc(ref)))
        t_got = unicodedata.normalize("NFC", codec.decode(greedy_ctc(got)))
        print(f"line {x.shape[3]:4d} px wide -> T {got.shape[3]:3d}, largest difference {diff:.2e}, text '{t_got}' "
              f"(drawn: '{text}')")
        if t_ref != t_got:
            sys.exit(f"the two decode differently: torch '{t_ref}', onnx '{t_got}'")
        shown.append((text, t_got))
    print(f"largest difference of all lines: {worst:.2e} (limit {args.max_difference})")
    if worst >= args.max_difference:
        sys.exit("ONNX and PyTorch do not agree")
    # a recognition that is wrong everywhere would mean the input is not what the network expects: say so loudly
    right = sum(1 for a, b in shown if a.lower() == b.lower())
    print(f"lines read back exactly (ignoring case): {right} of {len(shown)}")
    if right == 0:
        print("::warning::no rendered line was read back exactly: look at the texts above before using the model")

    marks = [f"U+{c:04X}" for c in KEEP_MARKS if chr(c) in info["codec"]]
    codec_path = os.path.join(args.outdir, args.name + "-codec.json")
    doc = {
        "format": 1,
        "model": args.name,
        "source": "kraken PP-OCRv6 small, small-models-for-glam/kraken-ppocrv6-small (Zenodo 10.5281/zenodo.21788405)",
        "license": LICENSE_OF_THE_MODEL,
        "variant": info["variant"],
        "parameters": n_params,
        "input": {"layout": "NCHW", "channels": 3, "height": args.height, "padding": args.padding,
                  "value": "1 - pixel / 255 (white is 0)", "resize": "Lanczos to the height, width follows"},
        "output": {"name": "logits", "layout": "(1, classes, 1, T)", "classes": info["num_classes"],
                   "blank": 0, "stepWidthPx": 8},
        "decode": "greedy CTC (argmax, collapse repeats, drop blank 0), labels to graphemes by longest match, then NFC",
        "unicodeForm": "NFD with combining marks; keep U+0300 to U+0303, U+0308 and U+0327",
        "combiningMarksInCodec": marks,
        "codec": info["codec"],
    }
    with open(codec_path, "w", encoding="utf-8") as fh:
        json.dump(doc, fh, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
        fh.write("\n")
    print(f"combining marks of the list in the codec: {marks}")
    print(f"{onnx_path}: {os.path.getsize(onnx_path):,} bytes; {codec_path}: {os.path.getsize(codec_path):,} bytes")


if __name__ == "__main__":
    main()
