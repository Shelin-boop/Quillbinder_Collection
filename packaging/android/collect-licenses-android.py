#!/usr/bin/env python3
"""Writes resources/licenses-android/ (Help > Third-Party Licenses of the Android program; 2026-11-14).

The APK ships other files than the AppImage and the Windows program: The Qt Company's build of Qt 6.12.1 for Android
with the third-party code compiled into its libraries, Hunspell linked into the program, OpenSSL (for https),
the C++ runtime of Android's NDK, the Java libraries Qt's Android part needs, and the spelling dictionaries. The list
has the format of resources/licenses/index.json and is compiled into the Android program under the same resource
names (packaging/android/android-resources.py writes the resource file at build time).

It is made from the Windows list (resources/licenses-windows, the same Qt version and the same Qt PDF 6.140.0, a part
of its own with the source of Qt WebEngine since Windows 0.93.1: the texts of Qt's parts, of Qt PDF's parts, of
Hunspell, Monocypher and the dictionaries are the same files) and from a folder with the texts only
the Android build needs:

  <extra>/forkfd/qt_attribution.json, LICENSE      qtbase v6.12.1, src/3rdparty/forkfd
  <extra>/libtiff/qt_attribution.json, COPYRIGHT   qtimageformats v6.12.1, src/3rdparty/libtiff
  <extra>/libcxx/LICENSE.TXT                       llvm-project llvmorg-21.1.0, libcxx (NDK r30's C++ runtime is LLVM 21;
                                                   the text is the same as 18.1.8's of NDK r27c)
  <extra>/openssl/LICENSE.txt                      openssl openssl-3.5.9
  (all from raw.githubusercontent.com/<project>/<repository>/<tag>/<path>)

Which of Qt's third-party parts the Android build carries is Qt's own statement: the SBOM that comes with the build
(<Qt for Android>/sbom/qtbase-6.12.0.spdx and qtimageformats-6.12.0.spdx; the Android workflow keeps the package
names as qt-sbom.txt). SBOM below is that list (run 20 of the workflow, 2026-11-14); the script stops when an entry
of it has no text here. Against the Windows list: no zlib inside Qt and none linked in (Android's own libz is used,
a part of the system), no Direct3D memory allocator, no Wintab, no Microsoft runtime; forkfd and libtiff are added.

  packaging/android/collect-licenses-android.py <extra>
"""
import json
import os
import shutil
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
WIN = os.path.join(ROOT, "resources", "licenses-windows")
OUT = os.path.join(ROOT, "resources", "licenses-android")
EXTRA = sys.argv[1]
QT_VERSION = "6.12.1"
SOURCES = "https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources"

# Qt's own list of the third-party parts in its Android libraries the APK carries (the Ids of qt_attribution.json;
# from the SBOM's package names "<Module>_Attribution_<id>" and "Bundled<Library>")
SBOM = [
    "pcre2", "pcre2-sljit", "unicode-character-database", "unicode-cldr", "tlexpected", "siphash", "blake2", "md4",
    "md5", "sha1", "sha3_endian", "sha3_keccak", "rfc6234", "tinycbor", "doubleconversion", "easing", "forkfd",
    "tika-mimetypes", "libpng", "libjpeg", "freetype", "freetype-zlib", "freetype-bdf", "freetype-pcf", "harfbuzz-ng",
    "psl-data", "libpsl", "rhi-miniengine-d3d12-mipmap", "opengl-headers", "opengl-es2-headers", "grayraster",
    "smooth-scaling-algorithm", "xserverhelper", "aglfn", "xsvg", "vulkanmemoryallocator", "emoji-segmenter",
    "webgradients", "icc-srgb-color-profile", "md4c", "vulkan-xml-spec", "libtiff", "libwebp",
]
NOT_ON_ANDROID = {"qt-zlib.txt", "qt-d3d12memoryallocator.txt", "qt-wintab.txt", "zlib.txt", "msvc-runtime.txt"}
WHERE = {  # the Windows list's places -> the APK's
    "Qt6Core.dll": "libQt6Core (inside the APK)",
    "Qt6Gui.dll (and the Windows platform plugin qwindows.dll, image plugins)":
        "libQt6Gui (inside the APK; and the Android platform plugin, the image plugins)",
    "Qt6Network.dll": "libQt6Network (inside the APK)",
    "Qt6Svg.dll, qsvg.dll, qsvgicon.dll": "libQt6Svg and the SVG plugins (inside the APK)",
    "the image plugin qwebp.dll": "the image plugin for WebP (inside the APK)",
    "Qt6Pdf.dll (showing PDF files)": "libQt6Pdf (inside the APK; showing PDF files)",
    # Android 0.93.2 (to-do item 48): Qt PDF 6.140.0 is a part of its own since Linux and Windows 0.93.1
    "Qt6Pdf.dll, Qt6PdfWidgets.dll (beside the program) and the image plugin qpdf.dll":
        "libQt6Pdf, libQt6PdfWidgets and the image plugin for PDF (libraries inside the APK)",
    "spelling (linked into quillbinder.exe)": "spelling (linked into the program's library)",
    "the check of package signatures (Ed25519), compiled into quillbinder.exe":
        "the check of package signatures (Ed25519), compiled into the program's library",
    "dictionaries/en_US.aff, en_US.dic": "en_US.aff, en_US.dic (the APK's assets, copied into the app's data folder)",
    "dictionaries/en_GB.aff, en_GB.dic": "en_GB.aff, en_GB.dic (the APK's assets, copied into the app's data folder)",
    "dictionaries/de_DE, de_AT, de_CH (.aff, .dic)":
        "de_DE, de_AT, de_CH (.aff, .dic; the APK's assets, copied into the app's data folder)",
}


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read().replace("\r\n", "\n")


def main():
    win = json.loads(read(os.path.join(WIN, "index.json")))
    if os.path.isdir(OUT):
        shutil.rmtree(OUT)
    os.makedirs(os.path.join(OUT, "texts"))
    texts = {}

    def add_text(name, body):
        texts[name] = body.rstrip() + "\n"
        return name

    def win_text(name):
        return add_text(name, read(os.path.join(WIN, "texts", name)))

    qtsrc = f"qt-everywhere-src-{QT_VERSION}.tar.xz"
    components = []
    have = set()
    for c in win["components"]:
        if c["text"] in NOT_ON_ANDROID:
            continue
        c = dict(c)
        if c["text"] == "qt.txt":
            c["where"] = ("libQt6Core, libQt6Gui, libQt6Widgets, libQt6Network, libQt6Xml, libQt6Svg, libQt6PrintSupport "
                          "(libraries inside the APK; Qt PDF: below), Qt's plugins (the Android platform "
                          "plugin, styles, image formats, icon engine, TLS, network information) and Qt's Java part (in "
                          "the APK's classes). The libraries are The Qt Company's own build for Android, unchanged; each "
                          "can be replaced by another build of the same version (an APK is a ZIP archive: the library "
                          "replaced, the archive aligned and signed again), or the APK built anew from the source.")
        elif c.get("where") in WHERE:
            c["where"] = WHERE[c["where"]]
        elif any(word in c.get("where", "") for word in (".dll", ".exe", "Windows")):
            sys.exit(f"a place of the Windows list has no Android wording: {c['name']}: {c['where']}")
        win_text(c["text"])
        if c["text"].startswith("qt-") and c["text"] != "qt-pdf.txt":  # (Qt PDF itself is a module, not a part of Qt's)
            have.add(c["text"][3:-4])
        components.append(c)

    def comp(after, name, version, license, where, text, source="", sourceFile=""):
        c = {"name": name, "version": version, "license": license, "where": where, "text": text}
        if source:
            c["source"] = source
        if sourceFile:
            c["sourceFile"] = sourceFile
        at = next(i for i, x in enumerate(components) if x["text"] == after) + 1
        components.insert(at, c)

    # ---- Qt's parts only the Android build has
    for pid, where, license_file, after in (("forkfd", "libQt6Core (inside the APK)", "LICENSE", "qt-tlexpected.txt"),
                                            ("libtiff", "the image plugin for TIFF (inside the APK)", "COPYRIGHT", "qt-libwebp.txt")):
        item = json.loads(read(os.path.join(EXTRA, pid, "qt_attribution.json")), strict=False)
        lines = [item["Name"]]
        if item.get("Version"):
            lines.append("Version: " + item["Version"])
        if item.get("Homepage"):
            lines.append("Homepage: " + item["Homepage"])
        lines += ["", item["Copyright"], "", read(os.path.join(EXTRA, pid, license_file))]
        comp(after, f"{item['Name']} (in Qt)", item.get("Version", ""), item["License"], where,
             add_text(f"qt-{pid}.txt", "\n".join(lines)), f"the Qt source, file {qtsrc}", qtsrc)
        have.add(pid)
    missing = [p for p in SBOM if p not in have]
    extra = sorted(have - set(SBOM))
    if missing:
        sys.exit("Qt's list of its Android build names parts without a text here: " + ", ".join(missing))
    if extra:
        sys.exit("parts listed here that Qt's list of its Android build does not name: " + ", ".join(extra))

    # ---- OpenSSL, the NDK's C++ runtime, the Java libraries
    comp("hunspell.txt", "OpenSSL", "3.5.9", "Apache-2.0 (Apache License 2.0)",
         "libcrypto_3.so, libssl_3.so inside the APK (secure connections: File > Import > Web Page). Built from "
         "OpenSSL's source without changes (packaging/android/android-openssl.sh).",
         add_text("openssl.txt", "OpenSSL 3.5.9\nCopyright (c) 1998-2026 The OpenSSL Project Authors\n"
                  "Copyright (c) 1995-1998 Eric A. Young, Tim J. Hudson\nhttps://www.openssl-library.org\n\n"
                  + read(os.path.join(EXTRA, "openssl", "LICENSE.txt"))),
         f"{SOURCES}, file openssl-3.5.9.tar.gz (also https://github.com/openssl/openssl, tag openssl-3.5.9)",
         "openssl-3.5.9.tar.gz")
    comp("openssl.txt", "LLVM libc++ (the C++ runtime of Android's NDK)", "NDK r30 (LLVM 21)",
         "Apache-2.0 WITH LLVM-exception (older parts: University of Illinois/NCSA Open Source License, MIT License)",
         "libc++_shared.so inside the APK, unchanged from the NDK",
         add_text("libcxx.txt", "libc++ of the LLVM Project, as it comes with Android's NDK r30\nhttps://libcxx.llvm.org\n\n"
                  + read(os.path.join(EXTRA, "libcxx", "LICENSE.TXT"))),
         "https://android.googlesource.com/toolchain/llvm-project (the NDK's LLVM); https://github.com/llvm/llvm-project")
    apache = ("\n\nLicensed under the Apache License, Version 2.0 (the full text is listed under the full texts of "
              "common licenses, \"Apache-2.0\").\n")
    comp("libcxx.txt", "AndroidX libraries (androidx.core 1.17.0 and the libraries it needs)", "",
         "Apache-2.0 (Apache License 2.0)",
         "the APK's Java part (classes.dex): Qt's Android part hands files to other apps with them",
         add_text("androidx.txt", "AndroidX (Jetpack) libraries\nCopyright (C) The Android Open Source Project\n"
                  "https://developer.android.com/jetpack/androidx\n\n"
                  "androidx.core:core 1.17.0, core-viewtree 1.0.0; androidx.annotation:annotation 1.8.1, "
                  "annotation-experimental 1.4.1; androidx.arch.core:core-common 2.2.0, core-runtime 2.2.0; "
                  "androidx.collection:collection 1.4.2; androidx.concurrent:concurrent-futures 1.1.0; "
                  "androidx.interpolator:interpolator 1.0.0; androidx.lifecycle:lifecycle-common 2.6.2, "
                  "lifecycle-runtime 2.6.2; androidx.profileinstaller:profileinstaller 1.3.0; "
                  "androidx.startup:startup-runtime 1.1.1; androidx.tracing:tracing 1.2.0; "
                  "androidx.versionedparcelable:versionedparcelable 1.1.1" + apache),
         "https://android.googlesource.com/platform/frameworks/support (Google's Maven repository: https://maven.google.com)")
    comp("androidx.txt", "Kotlin standard library, kotlinx.coroutines, JetBrains annotations", "2.2.10, 1.8.1, 23.0.0",
         "Apache-2.0 (Apache License 2.0)", "the APK's Java part (classes.dex): the AndroidX libraries need them",
         add_text("kotlin.txt", "Kotlin standard library 2.2.10 (org.jetbrains.kotlin:kotlin-stdlib)\n"
                  "kotlinx.coroutines 1.8.1 (org.jetbrains.kotlinx:kotlinx-coroutines-core, -android)\n"
                  "JetBrains annotations 23.0.0 (org.jetbrains:annotations)\n"
                  "Copyright (C) JetBrains s.r.o. and Kotlin Programming Language contributors\nhttps://kotlinlang.org"
                  + apache),
         "https://github.com/JetBrains/kotlin, https://github.com/Kotlin/kotlinx.coroutines, "
         "https://github.com/JetBrains/java-annotations")
    comp("kotlin.txt", "Guava ListenableFuture, JSpecify annotations", "1.0, 1.0.0", "Apache-2.0 (Apache License 2.0)",
         "the APK's Java part (classes.dex): the AndroidX libraries need them",
         add_text("guava-jspecify.txt", "Guava ListenableFuture 1.0 (com.google.guava:listenablefuture)\n"
                  "Copyright (C) The Guava Authors\nhttps://github.com/google/guava\n\n"
                  "JSpecify annotations 1.0.0 (org.jspecify:jspecify)\nCopyright (C) The JSpecify Authors\n"
                  "https://jspecify.dev" + apache),
         "https://github.com/google/guava, https://github.com/jspecify/jspecify")

    common = []
    for c in win["commonLicenses"]:
        common.append({"name": c["name"], "text": win_text(c["text"])})
    index = {
        "note": "Made by packaging/android/collect-licenses-android.py from the Windows list (the same Qt " + QT_VERSION
                + "), Qt's own list of the parts of its Android build (its SBOM), OpenSSL 3.5.9, the NDK's C++ runtime "
                "and the Java libraries Gradle put into the APK (sources pinned in the script).",
        "components": components,
        "commonLicenses": common,
    }
    for name, body in texts.items():
        with open(os.path.join(OUT, "texts", name), "w", encoding="utf-8", newline="\n") as f:
            f.write(body)
    with open(os.path.join(OUT, "index.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(index, f, indent=1, ensure_ascii=False)
        f.write("\n")
    print(f"{len(components)} components, {len(common)} common licenses, {len(texts)} text files")


main()
