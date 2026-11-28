#!/usr/bin/env python3
"""The designed voices of Read Aloud (the job "voices" of .github/workflows/models-5.yml, 0.99.0).

  design-voices.py make <language> <design-voices binary> <talker.gguf> <tokenizer.gguf> <out dir>
      designs 6 voices x 3 styles = 18 clips in that language with Qwen3-TTS VoiceDesign (tools/models/design-voices.cpp
      loads the model once), each clip reading the same reference text of the language; checks every clip (length,
      not silent, no clipped run); a clip that fails is designed again with other seeds (three tries in all), then the run fails; writes
      <out dir>/read-aloud-voices-<language>.zip (36 files and voices.json) and the same files in <out dir>/<language>/.
  design-voices.py check <wav file>...
      the clip check alone (prints OK or the reason for each file; exit code 1 when one fails).

The reference texts were written for this project (one neutral passage per language, 25 to 35 words); the voices are made
from descriptions only: no real person's voice, no recording of anyone.
"""
import array
import json
import math
import os
import subprocess
import sys
import wave
import zipfile

VOICES = {
    "female1": "Female, young adult, warm clear mid-range voice",
    "female2": "Female, middle-aged, lower pitch, slightly husky voice",
    "female3": "Female, older adult, calm low voice",
    "male1": "Male, adult, deep warm baritone voice",
    "male2": "Male, young adult, lighter higher pitch, friendly voice",
    "male3": "Male, older adult, slightly gravelly, slow voice",
}
STYLES = {
    "neutral": "natural even pacing, neutral narration",
    "calm": "calm and soothing, soft, slow relaxed pace",
    "action": "energetic, urgent, fast pace, tense and intense",
}
LANGUAGE_NAMES = {"en": "english", "de": "german", "es": "spanish", "fr": "french", "pt": "portuguese", "it": "italian", "ru": "russian"}
TEXTS = {
    "en": "The morning light moved slowly across the quiet kitchen table, where a cup of tea was cooling beside an open notebook, and for a moment nothing in the house seemed to need an answer.",
    "de": "Das Licht des Morgens wanderte langsam über den stillen Küchentisch, auf dem neben einem aufgeschlagenen Heft eine Tasse Tee kalt wurde, und für einen Augenblick schien im ganzen Haus nichts eine Antwort zu verlangen.",
    "es": "La luz de la mañana se deslizaba despacio sobre la mesa silenciosa, donde un té se enfriaba junto a un cuaderno abierto, y por un momento nada en la casa parecía exigir respuesta.",
    "fr": "La lumière du matin glissait lentement sur la table silencieuse, où une tasse de thé refroidissait près d'un cahier ouvert, et pendant un instant rien dans la maison ne semblait réclamer de réponse.",
    "pt": "A luz da manhã deslizava devagar sobre a mesa silenciosa da cozinha, onde uma xícara de chá esfriava ao lado de um caderno aberto, e por um instante nada na casa parecia exigir resposta.",
    "it": "La luce del mattino scivolava piano sul tavolo silenzioso della cucina, dove una tazza di tè si raffreddava accanto a un quaderno aperto, e per un attimo niente in casa sembrava chiedere una risposta.",
    "ru": "Утренний свет медленно скользил по тихому кухонному столу, где рядом с раскрытой тетрадью остывала чашка чая, и на мгновение казалось, что в доме ничто не требует ответа.",
}
MIN_SECONDS, MAX_SECONDS = 5.0, 25.0  # a calm, slow voice reads the passage in up to about 25 s
RMS_FLOOR = 0.01          # of full scale; speech is far above
CLIP_RUN = 10             # this many samples in a row at the limit: clipped
MODEL = "qwen-talker-1.7b-voicedesign-Q8_0.gguf"


def clip_problem(path):
    """None for a good clip, else the reason (mono 24 kHz 16 bit, 5 to 25 s, not silent, no clipping run)."""
    try:
        with wave.open(path, "rb") as w:
            ch, width, rate, n = w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()
            raw = w.readframes(n)
    except (wave.Error, EOFError, OSError) as e:
        return f"not a readable WAV file ({e})"
    if ch != 1 or width != 2 or rate != 24000:
        return f"not mono 24 kHz 16 bit ({ch} channels, {width * 8} bit, {rate} Hz)"
    secs = n / rate
    if not MIN_SECONDS <= secs <= MAX_SECONDS:
        return f"{secs:.1f} s long (wanted {MIN_SECONDS:g} to {MAX_SECONDS:g} s)"
    s = array.array("h")
    s.frombytes(raw)
    if sys.byteorder == "big":
        s.byteswap()
    rms = math.sqrt(sum(x * x for x in s) / len(s)) / 32768.0
    if rms < RMS_FLOOR:
        return f"nearly silent (RMS {rms:.4f})"
    run = 0
    for x in s:
        run = run + 1 if abs(x) >= 32700 else 0
        if run >= CLIP_RUN:
            return "clipped (a run of samples at full scale)"
    return None


def names():
    return [f"{v}-{s}" for v in VOICES for s in STYLES]


def description(name):
    v, s = name.split("-")
    return f"{VOICES[v]}, {STYLES[s]}"


def run_tool(tool, talker, codec, lang, wanted, out, seed):
    lst = os.path.join(out, "list.tsv")
    with open(lst, "w", encoding="utf-8") as f:
        for n in wanted:
            f.write(f"{n}\t{description(n)}\t{TEXTS[lang]}\n")
    # the tool sits next to the engine's libraries (the compute modules are found beside the program)
    subprocess.run([tool, talker, codec, LANGUAGE_NAMES[lang], lst, out, str(seed)], check=False, timeout=3 * 3600)


def make(lang, tool, talker, codec, out):
    if lang not in TEXTS:
        sys.exit(f"no reference text for {lang}")
    os.makedirs(out, exist_ok=True)
    todo = names()
    for attempt, seed in enumerate((1000, 2000, 3000)):
        for n in todo:
            p = os.path.join(out, n + ".wav")
            if os.path.exists(p):
                os.remove(p)
        run_tool(tool, talker, codec, lang, todo, out, seed)
        bad = []
        for n in todo:
            p = os.path.join(out, n + ".wav")
            why = clip_problem(p) if os.path.isfile(p) else "was not made"
            print(f"{lang} {n}: {'OK' if why is None else why}")
            if why is not None:
                bad.append(n)
        # keep the good ones of the first round: only the failed clips are designed again
        if not bad:
            break
        if attempt == 2:
            sys.exit(f"clips that failed three times: {' '.join(bad)}")
        todo = bad
    folder = os.path.join(out, lang)
    os.makedirs(folder, exist_ok=True)
    for n in names():
        os.replace(os.path.join(out, n + ".wav"), os.path.join(folder, n + ".wav"))
        with open(os.path.join(folder, n + ".txt"), "w", encoding="utf-8") as f:
            f.write(TEXTS[lang] + "\n")
    meta = {
        "language": lang,
        "model": MODEL,
        "modelLicense": "Apache-2.0",
        "note": "Designed for Quillbinder; no real person's voice. Each clip was made from a written description, not from a recording.",
        "text": TEXTS[lang],
        "voices": {v: {"description": d} for v, d in VOICES.items()},
        "styles": STYLES,
        "files": names(),
    }
    with open(os.path.join(folder, "voices.json"), "w", encoding="utf-8") as f:
        json.dump(meta, f, ensure_ascii=False, indent=1)
        f.write("\n")
    zp = os.path.join(out, f"read-aloud-voices-{lang}.zip")
    with zipfile.ZipFile(zp, "w", zipfile.ZIP_DEFLATED) as z:
        for fn in sorted(os.listdir(folder)):
            z.write(os.path.join(folder, fn), fn)
    print(f"{zp}: {os.path.getsize(zp)} bytes, {len(os.listdir(folder))} files")


def main():
    if len(sys.argv) >= 3 and sys.argv[1] == "check":
        bad = 0
        for p in sys.argv[2:]:
            why = clip_problem(p)
            print(f"{p}: {'OK' if why is None else why}")
            bad += why is not None
        sys.exit(1 if bad else 0)
    if len(sys.argv) == 7 and sys.argv[1] == "make":
        make(*sys.argv[2:])
        return
    sys.exit(__doc__)


if __name__ == "__main__":
    main()
