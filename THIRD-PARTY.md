# Third-party parts

Quillbinder's own source code and files are under the Apache License 2.0 (`LICENSE`), since 0.80.0:
anyone may use, copy, change and pass them on, with or without fee, keeping the license and the
`NOTICE` file, which names the project and its source. Versions up to 0.70.0 were published under the
Zero-Clause BSD license (0BSD); copies of those versions stay under it.

The program uses, and its AppImage carries, parts of other projects under their own licenses. None
of them is changed by the Apache License 2.0 of Quillbinder. The full license texts of everything the
AppImage carries are in `resources/licenses` (made by `tools/collect-licenses.py` from the Ubuntu
24.04 packages the libraries come from), in the AppImage under `usr/share/doc/quillbinder/licenses`,
and in the program under **Help > Third-Party Licenses** (also a button of **Help > About**).

| Part | Where | License |
|------|-------|---------|
| Qt 6.12.1 (Core, Gui, Widgets, Network, PrintSupport, Svg, Xml, DBus, Wayland and XCB platform parts, image format plugins; The Qt Company's own build for Linux, since 0.80.0; 6.12.0 before 0.93.0), with the parts Qt builds in (Qt's attribution files) and ICU 73.2 as Qt ships it | linked dynamically; carried by the AppImage | LGPL-3.0 (also available under GPL-2.0 / GPL-3.0); the parts inside Qt under their own licenses (MIT, BSD, Zlib, FTL, Unicode and others), ICU Unicode-3.0 |
| Qt PDF 6.140.0 (Pdf, PdfWidgets and its image plugin; since Qt 6.12 a Qt Extensions release of Qt WebEngine and Qt PDF with a version of its own, The Qt Company's build for Qt 6.12) | linked dynamically; carried by the AppImage | LGPL-3.0 (also available under GPL-2.0 / GPL-3.0); the parts inside it (PDFium and the parts of Chromium it uses) under their own licenses (BSD, Apache-2.0, FTL, Unicode, MIT and others) |
| Qt Multimedia 6.12.1 with Qt Quick, Qt QML, Qt OpenGL and Qt Concurrent (what its FFmpeg backend needs), Qt's TLS plugin | the add-ons "Media Playback", "Speech Transcription" and "Web Page Import" | LGPL-3.0 |
| Hunspell (spell checking) | linked dynamically; carried by the AppImage | tri-license MPL-1.1 / GPL-2.0-or-later / LGPL-2.1-or-later |
| The other libraries the AppImage carries (Hunspell's and the system parts linuxdeploy takes from Ubuntu 24.04: libxkbcommon, Wayland and XCB libraries and others) | carried by the AppImage | their own licenses (MIT, BSD, Zlib, LGPL and others); each with its Ubuntu copyright file in `resources/licenses` |
| AppImage type2-runtime (the start code of the AppImage file), with musl libc, libfuse, squashfuse, zstd and zlib linked into it | the beginning of the AppImage file | MIT; musl MIT, libfuse LGPL-2.1, squashfuse BSD-2-Clause, zstd BSD-3-Clause or GPL-2.0, zlib Zlib (`packaging/linux/licenses`) |
| Breeze icons (a subset; the microphone, record and list icons added in 0.61) | `resources/icons/` | LGPL-3.0-or-later, see `resources/icons/COPYING-BREEZE` |
| Monocypher 4.0.3 (only its Ed25519 check of package signatures, 0.65.0; unchanged) | `src/core/third_party/monocypher/`, compiled into the program | BSD-2-Clause (dual-licensed BSD-2-Clause or CC0-1.0), see `src/core/third_party/monocypher/LICENCE.md` |
| Botan 3.14.1 (a minimized build with the modules RNP needs; static, 0.99.0), RNP 0.18.1 with its S-expression library sexpp, json-c 0.19, bzip2 1.0.8 and zlib 1.4 | the encryption built into the program: linked statically (`tools/get-crypto.sh`, the pack `crypto`) | Botan BSD-2-Clause; RNP BSD-2-Clause, BSD-3-Clause, Apache-2.0 and MIT parts, with the OCB license text of its source; sexpp MIT; json-c MIT; bzip2 its BSD-style license; zlib the zlib license (`packaging/linux/licenses/crypto-*.txt`) |
| QtKeychain 0.18.0 | the program: keeps the passwords of the built-in encryption in the system's key store | BSD-3-Clause (`packaging/linux/licenses/qtkeychain.txt`) |
| FFmpeg 9.1 (decoding only, built without GPL parts; 9.0.2 before 0.93.0, 6.1.1 before 0.80.0) | the add-ons "Media Playback" (free formats only) and "Extra Video and Audio Formats" (every decoder; `addons/media-*`, built by `tools/build-ffmpeg.sh`) | LGPL-2.1-or-later; the add-ons carry their license texts |
| whisper.cpp and ggml (speech recognition), built by `tools/get-whisper.sh` | the add-on "Speech Transcription" | MIT (`speech/licenses/whisper.cpp-MIT.txt` in the add-on) |
| ONNX Runtime 1.32.0 (Microsoft's prebuilt Linux x64 archive, checked by its SHA-256; `tools/get-onnxruntime.sh`, the pack `onnxruntime`; 0.99.0) | an engine package of the AI add-ons; the program loads it at run time (`dlopen`), it is not linked | MIT (license text and third-party notices of the archive in the pack) |
| ggml 1.1.1, whisper.cpp 1.9.6, llama.cpp b12808 (libllama and libmtmd only), qwentts.cpp (commit 2edb35f) and Sonic (commit b93885d), built together by `tools/get-ai-engine.sh` (the pack `aiengine`; 0.99.0); ggml.h differs from the tag in one line (the length of a tensor's name, 128) | the engine packages of the AI add-ons (speech recognition, speech synthesis, changing the speed of speech); shared libraries, loaded at run time | MIT (ggml, whisper.cpp, llama.cpp, qwentts.cpp); Sonic Apache-2.0; the pack carries the license texts. libmtmd holds stb_image (MIT or public domain), miniaudio (public domain or MIT-0), subprocess.h (Unlicense), xxHash (BSD-2-Clause) and SHA-1/SHA-256 code (public domain), the CPU modules llamafile's sgemm (MIT): their texts are in the package "AI Engine" (`addons/ai-engine`, 0.99.0) |
| Whisper models (converted for whisper.cpp) | not carried: downloaded by the program when the writer chooses one (Speech Transcription) | MIT |
| Test recording of the speech add-on (J. F. Kennedy, inaugural address, 1961; as whisper.cpp ships it) | the add-on "Speech Transcription" | public domain (a work of the United States government) |
| Name lists of the Name Generator | the add-on "Name Generator", made from the Python package Faker by `tools/make-name-lists.py` | MIT (`names/LICENSE-Faker.txt` in the add-on) |
| linuxdeploy, linuxdeploy-plugin-qt, appimagetool | used to build the AppImage, not part of the program | MIT |

The source code of every one of these parts, exactly in the version the AppImage and the add-ons
carry, is in the release "Third-party source code" of the project page:
https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources (the Qt Project's
source release of Qt 6.12.1 and, since 0.93.1, the one of Qt WebEngine and Qt PDF 6.140.0 (Qt PDF's source, which
Qt's own source release no longer holds since Qt 6.12), one file per Ubuntu source package, FFmpeg with the patch and build scripts of
the add-ons, the AppImage runtime with the libraries it links in; `tools/collect-sources.py` makes them, the
release workflow publishes them). Help > Third-Party Licenses names the file for each library; Ubuntu's archive has the Ubuntu
packages as well (`apt-get source <package>=<version>`). The libraries inside the AppImage may be
replaced with other builds of the same version (extract the AppImage with `--appimage-extract`).

Scrivener and Scapple are names of products of Literature & Latte. Quillbinder is an independent
program, not made by or connected with Literature & Latte. It reads and writes their file formats so
that projects can move between the programs; it contains no program code of them.
