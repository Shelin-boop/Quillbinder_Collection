#!/usr/bin/env python3
"""Builds .qbpkg packages (developer tool; users never need this).

  make-package.py addon  [--platform windows-x64] <folder> <out.qbpkg>
      <folder>/manifest.json   id, name, version, description, author, whatsNew, minApp
      --platform: the system the add-on's libraries were built for (Windows add-ons, port W2;
      Linux add-ons name none)
      <folder>/payload/...     themes/, templates/, compile-formats/, plugins/, dictionaries/

  make-package.py core   --binary <program> --version X.Y.Z --runtime <id>
                         [--plugins <dir>] [--whats-new "text" ...] <out.qbpkg>

  make-package.py bundle --id <id> --name <name> --version X.Y [--description "text"]
                         <package.qbpkg> ... <out.qbpkg>

  make-package.py sign <package.qbpkg> ...
      signs finished packages in place (0.70.0, audit A S-03: the release workflow builds without the key
      and signs in a job of its own that runs no third-party code); a bundle's parts are signed too

  make-package.py certify --name "<who holds the key>" [--issued yyyy-mm-dd] <add-on key: PEM file or 64 hex> <out.json>
      (0.91.3) the release key (QB_SIGNING_KEY_FILE) vouches for an add-on key: a certificate for add-ons only

  make-package.py verify --release-key <public PEM> <package.qbpkg> ...
      (0.91.3) checks every signature as the program does: a program update (and a bundle holding one) only with the
      release key itself; an add-on also with an add-on key whose certificate the release key signed

The file list with SHA-256 sums is written into the manifest here.

Signatures (0.65.0): with QB_SIGNING_KEY_FILE=<Ed25519 private key, PEM> every package gets manifest.sig,
the key's signature of manifest.json (made with openssl). The release workflow sets it from the
secret QB_SIGNING_KEY; without it packages are made unsigned (the program then warns, and refuses
unsigned program updates). A bundle's parts carry their own signatures.

Add-on keys (0.91.3, the writer's decision of 2026-11-15): with QB_ADDON_KEY_FILE=<Ed25519 private key, PEM> and
QB_ADDON_CERT=<its certificate from "certify"> add-ons, and bundles of add-ons only, are signed with the add-on key and
carry the certificate in their manifest.sig; program updates, and bundles that hold one, keep the release key.
"""
import argparse, base64, hashlib, json, os, re, subprocess, sys, tempfile, zipfile


def file_entry(path, rel, executable=False):
    data = open(path, "rb").read()
    e = {"path": rel, "sha256": hashlib.sha256(data).hexdigest(), "size": len(data)}
    if executable:
        e["executable"] = True
    return e, data


ED25519_DER_PREFIX = bytes.fromhex("302a300506032b6570032100")


def key_id(pub):
    return hashlib.sha256(pub).hexdigest()[:16]


def public_of(keyfile):
    """the 32 bytes of the public half of an Ed25519 key file (private or public PEM)"""
    args = ["openssl", "pkey", "-in", keyfile, "-pubout", "-outform", "DER"]
    if b"PUBLIC KEY" in open(keyfile, "rb").read():
        args.insert(2, "-pubin")
    pub = subprocess.run(args, check=True, capture_output=True).stdout[-32:]
    if len(pub) != 32:
        sys.exit("make-package.py: %s is not an Ed25519 key" % keyfile)
    return pub


def sign_raw(keyfile, data):
    with tempfile.TemporaryDirectory() as t:
        msg, sig = os.path.join(t, "message"), os.path.join(t, "signature")
        with open(msg, "wb") as f:
            f.write(data)
        subprocess.run(["openssl", "pkeyutl", "-sign", "-rawin", "-inkey", keyfile, "-in", msg, "-out", sig], check=True)
        raw = open(sig, "rb").read()
    if len(raw) != 64:
        sys.exit("make-package.py: %s is not an Ed25519 key" % keyfile)
    return raw


def verify_raw(pub, data, sig):
    with tempfile.TemporaryDirectory() as t:
        pem, msg, sf = os.path.join(t, "key.pem"), os.path.join(t, "message"), os.path.join(t, "signature")
        with open(pem, "w") as f:
            f.write("-----BEGIN PUBLIC KEY-----\n%s\n-----END PUBLIC KEY-----\n"
                    % base64.b64encode(ED25519_DER_PREFIX + pub).decode())
        open(msg, "wb").write(data)
        open(sf, "wb").write(sig)
        return subprocess.run(["openssl", "pkeyutl", "-verify", "-pubin", "-inkey", pem, "-rawin", "-in", msg,
                               "-sigfile", sf], capture_output=True).returncode == 0


def certificate_text(role, pub, name, issued):
    """the text a release key signs to vouch for an add-on key (src/core/Signing.cpp, certificateText)"""
    return ("Quillbinder key certificate 1\nrole: %s\nkey: %s\nname: %s\nissued: %s\n"
            % (role, pub.hex(), name, issued)).encode("utf-8")


def addon_signer():
    """(key file, certificate) of the add-on key, or None"""
    key, cert = os.environ.get("QB_ADDON_KEY_FILE"), os.environ.get("QB_ADDON_CERT")
    if not key and not cert:
        return None
    if not (key and cert):
        sys.exit("make-package.py: QB_ADDON_KEY_FILE and QB_ADDON_CERT go together")
    c = json.load(open(cert))
    if c.get("role") != "addons" or c.get("key") != public_of(key).hex():
        sys.exit("make-package.py: QB_ADDON_CERT is not the certificate of the key in QB_ADDON_KEY_FILE")
    return key, c


def signature(data, program_update=True):
    """manifest.sig for these bytes, or None without a key. program_update: a program update or a bundle that holds one
    (only the release key); otherwise the add-on key when one is set (0.91.3), else the release key"""
    addon = None if program_update else addon_signer()
    if addon:
        key, cert = addon
        pub = public_of(key)
        return json.dumps({"algorithm": "ed25519", "key": key_id(pub), "signature": base64.b64encode(sign_raw(key, data)).decode(),
                           "signs": "manifest.json", "certificate": cert}, indent=2)
    key = os.environ.get("QB_SIGNING_KEY_FILE")
    if not key:
        return None
    raw, pub = sign_raw(key, data), public_of(key)
    return json.dumps({"algorithm": "ed25519", "key": key_id(pub),
                       "signature": base64.b64encode(raw).decode(), "signs": "manifest.json"}, indent=2)


def holds_program_update(manifest_bytes, zf=None):
    """a program update, or a bundle that holds one (looked at in its parts)"""
    import io
    m = json.loads(manifest_bytes)
    if m.get("kind") == "core":
        return True
    if m.get("kind") != "bundle" or zf is None:
        return False
    for n in zf.namelist():
        if n.startswith("packages/") and n.endswith(".qbpkg"):
            inner = zipfile.ZipFile(io.BytesIO(zf.read(n)))
            if holds_program_update(inner.read("manifest.json"), inner):
                return True
    return False


def write_package(out, manifest, payload, packages=()):
    if os.path.exists(out):
        os.remove(out)
    mdata = json.dumps(manifest, indent=2, ensure_ascii=False).encode("utf-8")
    core = manifest.get("kind") == "core" or any(
        holds_program_update(zipfile.ZipFile(__import__("io").BytesIO(d)).read("manifest.json"),
                             zipfile.ZipFile(__import__("io").BytesIO(d))) for _, d in packages)
    sig = signature(mdata, core)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        z.writestr("manifest.json", mdata)
        if sig:
            z.writestr("manifest.sig", sig)
        for rel, data in payload:
            z.writestr("payload/" + rel, data)
        for name, data in packages:
            z.writestr("packages/" + name, data, compress_type=zipfile.ZIP_STORED)
    print(f"{out}  ({os.path.getsize(out) / 1e6:.2f} MB)")


def cmake_var(name):
    """The first set(<name> "...") of CMakeLists.txt (the Linux values come first)."""
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    with open(os.path.join(root, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r'^set\(%s\s+"([^"]*)"\)' % name, f.read(), re.M)
    return m.group(1) if m else ""


def version_key(v):
    return [int(x) if x.isdigit() else 0 for x in v.split(".")]


def cmd_addon(a):
    manifest = json.load(open(os.path.join(a.folder, "manifest.json"), encoding="utf-8"))
    manifest.setdefault("format", 1)
    manifest["kind"] = "addon"
    if a.platform:
        manifest["platform"] = a.platform
    files, payload = [], []
    root = os.path.join(a.folder, "payload")
    for d, _, names in sorted(os.walk(root)):
        for n in sorted(names):
            p = os.path.join(d, n)
            rel = os.path.relpath(p, root).replace(os.sep, "/")
            e, data = file_entry(p, rel, executable=n.endswith(".so") or n.lower().endswith(".dll"))
            files.append(e)
            payload.append((rel, data))
    manifest["files"] = files
    # 0.80.0: Linux add-ons with libraries are built for one runtime (the Qt inside the AppImage): they name it,
    # and need the first program version that has it (older programs refuse them by that, newer ones by the runtime)
    libs = any(f["path"].endswith(".so") or ".so." in f["path"] for f in files)
    if libs and not (a.platform or "").startswith("windows"):
        manifest["runtime"] = cmake_var("APP_RUNTIME")
        since = cmake_var("APP_RUNTIME_SINCE")
        if since and version_key(manifest.get("minApp", "0")) < version_key(since):
            manifest["minApp"] = since
    write_package(a.out, manifest, payload)


def cmd_core(a):
    files, payload = [], []
    e, data = file_entry(a.binary, os.path.basename(a.binary), executable=True)
    files.append(e)
    payload.append((e["path"], data))
    if a.plugins:
        for n in sorted(os.listdir(a.plugins)):
            p = os.path.join(a.plugins, n)
            if os.path.isfile(p) and (n.endswith(".so") or n.lower().endswith(".dll") or n.endswith(".json")):
                e2, d2 = file_entry(p, "plugins/" + n, executable=n.endswith(".so") or n.lower().endswith(".dll"))
                files.append(e2)
                payload.append((e2["path"], d2))
    manifest = {"format": 1, "id": "core", "name": a.name, "version": a.version, "kind": "core",
                "runtime": a.runtime, "binary": os.path.basename(a.binary),
                "description": a.description, "whatsNew": a.whats_new or [], "files": files}
    write_package(a.out, manifest, payload)


def cmd_bundle(a):
    packages = [(os.path.basename(p), open(p, "rb").read()) for p in a.packages]
    manifest = {"format": 1, "id": a.id, "name": a.name, "version": a.version, "kind": "bundle",
                "description": a.description, "whatsNew": a.whats_new or [],
                "packages": [n for n, _ in packages]}
    write_package(a.out, manifest, [], packages)


def signed_bytes(data):
    """the package (bytes of a .qbpkg) with manifest.sig made anew, its parts (a bundle's packages) as well"""
    import io
    src = zipfile.ZipFile(io.BytesIO(data))
    manifest = src.read("manifest.json")
    sig = signature(manifest, holds_program_update(manifest, src))
    if not sig:
        sys.exit("make-package.py sign: QB_SIGNING_KEY_FILE is not set")
    out = io.BytesIO()
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        z.writestr("manifest.json", manifest)
        z.writestr("manifest.sig", sig)
        for info in src.infolist():
            if info.filename in ("manifest.json", "manifest.sig"):
                continue
            body = src.read(info.filename)
            if info.filename.startswith("packages/") and info.filename.endswith(".qbpkg"):
                z.writestr(info.filename, signed_bytes(body), compress_type=zipfile.ZIP_STORED)
            else:
                z.writestr(info, body, compress_type=info.compress_type)
    return out.getvalue()


def cmd_sign(a):
    for p in a.packages:
        data = signed_bytes(open(p, "rb").read())
        tmp = p + ".signing"
        with open(tmp, "wb") as f:
            f.write(data)
        os.replace(tmp, p)
        print(f"{p}  signed ({len(data) / 1e6:.2f} MB)")


def cmd_certify(a):
    key = os.environ.get("QB_SIGNING_KEY_FILE")
    if not key:
        sys.exit("make-package.py certify: QB_SIGNING_KEY_FILE (the release key) is not set")
    if re.fullmatch(r"[0-9a-fA-F]{64}", a.key):
        pub = bytes.fromhex(a.key)
    else:
        pub = public_of(a.key)
    name, issued = a.name.strip(), a.issued or __import__("datetime").datetime.now(__import__("datetime").timezone.utc).strftime("%Y-%m-%d")
    if not name or len(name) > 120 or "\n" in name or "\r" in name or not re.fullmatch(r"\d{4}-\d{2}-\d{2}", issued):
        sys.exit("make-package.py certify: a name of one line (at most 120 characters) and a date yyyy-mm-dd")
    release = public_of(key)
    sig = sign_raw(key, certificate_text("addons", pub, name, issued))
    cert = {"format": 1, "role": "addons", "key": pub.hex(), "name": name, "issued": issued, "by": key_id(release),
            "signature": base64.b64encode(sig).decode()}
    with open(a.out, "w") as f:
        json.dump(cert, f, indent=2)
    print(f"{a.out}: add-on key {key_id(pub)} ({name}), vouched for by the release key {key_id(release)}")


def check_package(data, name, release, out):
    """the program's rule for one package and its parts; appends lines to out; True when all is trusted"""
    import io
    z = zipfile.ZipFile(io.BytesIO(data))
    manifest = z.read("manifest.json")
    try:
        s = json.loads(z.read("manifest.sig"))
    except KeyError:
        out.append("NOT SIGNED: " + name)
        return False
    sig = base64.b64decode(s.get("signature", ""))
    ok, how = False, ""
    if s.get("algorithm") == "ed25519" and s.get("signs") == "manifest.json" and len(sig) == 64:
        if s.get("key") == key_id(release):
            ok, how = verify_raw(release, manifest, sig), "the release key"
        elif isinstance(s.get("certificate"), dict) and not holds_program_update(manifest, z):
            c = s["certificate"]
            pub = bytes.fromhex(c.get("key", "")) if re.fullmatch(r"[0-9a-f]{64}", c.get("key", "")) else b""
            csig = base64.b64decode(c.get("signature", ""))
            if (c.get("format") == 1 and c.get("role") == "addons" and len(pub) == 32 and key_id(pub) == s.get("key")
                    and c.get("by") == key_id(release) and len(csig) == 64
                    and verify_raw(release, certificate_text("addons", pub, c.get("name", ""), c.get("issued", "")), csig)):
                ok, how = verify_raw(pub, manifest, sig), "the add-on key '%s'" % c.get("name")
    out.append(("signed: %s (%s)" % (name, how)) if ok else ("NOT TRUSTED: " + name))
    for n in z.namelist():
        if n.startswith("packages/") and n.endswith(".qbpkg"):
            ok = check_package(z.read(n), name + " > " + n[len("packages/"):], release, out) and ok
    return ok


def cmd_verify(a):
    release = public_of(a.release_key)
    out, ok = [], True
    for p in a.packages:
        ok = check_package(open(p, "rb").read(), p, release, out) and ok
    print("\n".join(out))
    if not ok:
        sys.exit(1)


ap = argparse.ArgumentParser()
sub = ap.add_subparsers(dest="cmd", required=True)
s = sub.add_parser("addon"); s.add_argument("--platform"); s.add_argument("folder"); s.add_argument("out")
s.set_defaults(fn=cmd_addon)
s = sub.add_parser("core")
s.add_argument("--binary", required=True); s.add_argument("--version", required=True)
s.add_argument("--runtime", required=True); s.add_argument("--plugins")
s.add_argument("--name", default="Program update"); s.add_argument("--description", default="")
s.add_argument("--whats-new", action="append"); s.add_argument("out"); s.set_defaults(fn=cmd_core)
s = sub.add_parser("bundle")
s.add_argument("--id", required=True); s.add_argument("--name", required=True)
s.add_argument("--version", required=True); s.add_argument("--description", default="")
s.add_argument("--whats-new", action="append")
s.add_argument("packages", nargs="+"); s.add_argument("out"); s.set_defaults(fn=cmd_bundle)
s = sub.add_parser("sign")
s.add_argument("packages", nargs="+"); s.set_defaults(fn=cmd_sign)
s = sub.add_parser("certify")
s.add_argument("--name", required=True); s.add_argument("--issued"); s.add_argument("key"); s.add_argument("out")
s.set_defaults(fn=cmd_certify)
s = sub.add_parser("verify")
s.add_argument("--release-key", required=True); s.add_argument("packages", nargs="+"); s.set_defaults(fn=cmd_verify)
args = ap.parse_args()
args.fn(args)
