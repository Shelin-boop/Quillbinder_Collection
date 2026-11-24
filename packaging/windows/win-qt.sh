#!/usr/bin/env bash
# Qt for MSVC 2022 on GitHub's Windows runner (Windows 0.93.0): the one place where the Windows workflows
# (windows.yml, release-windows.yml) install Qt. Before 0.93.0 each workflow step did it itself.
#
#   packaging/windows/win-qt.sh <Qt version> <root folder, e.g. C:/Qt>
#
# Installs <root>/<version>/msvc2022_64 with Qt Multimedia (the add-on "Media Playback"), Qt Image Formats (WebP, as
# on Linux) and Qt PDF. aqtinstall 3.3.0 as for the Linux build (tools/get-qt.sh), which checks every archive against
# the checksums of download.qt.io.
#
# - Since Qt 6.11 Qt's repository keeps the Windows packages in one folder per compiler
#   (windows_x86/desktop/qt6_6120/qt6_6120_msvc2022_64/, seen on the runner 2026-11-11); aqtinstall 3.3.0 (its newest
#   release, 2025-06-02) looks in qt6_6120/qt6_6120/ and finds nothing, so it is given the folder name (its
#   "extension" for that architecture) before it runs.
# - Since Qt 6.12, Qt PDF is a "Qt Extensions" module with releases of its own (Qt WebEngine 6.140.0 for Qt 6.12),
#   listed only under the Qt version it was built with (6.12.0): when this version has none, it is taken from the
#   newest earlier patch release of the same Qt, its files only (--noarchives), as tools/get-qt.sh does on Linux.
set -euo pipefail
V=$1
ROOT=${2:-C:/Qt}
ARCH=win64_msvc2022_64
python -m pip install -q 'aqtinstall==3.3.0'
python - <<'PY'
import pathlib
import aqt.metadata as m
p = pathlib.Path(m.__file__)
s = p.read_text(encoding="utf-8")
mark = "# Quillbinder W4a: Qt 6.11+ folders"
if mark not in s:
    old = "    def extension_for_arch(architecture: str, is_version_ge_6: bool) -> str:\n"
    new = old + ("        if architecture.startswith('win64_') and is_version_ge_6:  " + mark + "\n"
                 "            return architecture[len('win64_'):]\n")
    assert old in s, "aqtinstall changed: extension_for_arch not found"
    p.write_text(s.replace(old, new, 1), encoding="utf-8")
print("aqtinstall patched:", p)
PY
D="$(cygpath -u "$ROOT")/$V/msvc2022_64"
if ! python -m aqt install-qt windows desktop "$V" $ARCH -O "$ROOT" -m qtmultimedia qtimageformats qtpdf; then
    echo "Qt $V with Qt PDF: not found together; Qt $V without it"
    python -m aqt install-qt windows desktop "$V" $ARCH -O "$ROOT" -m qtmultimedia qtimageformats
fi
test -f "$D/bin/Qt6Core.dll"
if [ ! -f "$D/lib/cmake/Qt6Pdf/Qt6PdfConfig.cmake" ]; then
    from=""
    T=$(mktemp -d)
    for p in $(seq $(( ${V##*.} - 1 )) -1 0); do
        f="${V%.*}.$p"
        if python -m aqt install-qt windows desktop "$f" $ARCH -O "$(cygpath -w "$T")" -m qtpdf --noarchives \
            && [ -f "$T/$f/msvc2022_64/lib/cmake/Qt6Pdf/Qt6PdfConfig.cmake" ]; then
            from=$f
            break
        fi
    done
    [ -n "$from" ] || { echo "no Qt PDF for Qt ${V%.*} in Qt's repository" >&2; exit 1; }
    cp -rn "$T/$from/msvc2022_64/." "$D/"
    rm -rf "$T"
    echo "Qt PDF taken from Qt $from's repository folder (the Qt Extensions release for Qt ${V%.*})"
fi
test -f "$D/lib/cmake/Qt6Pdf/Qt6PdfConfig.cmake"
ls "$D/plugins"
echo "Qt's FFmpeg libraries: $(ls "$D/bin" | grep -iE '^(av|sw)[a-z]*-?[0-9]*\.dll$' | tr '\n' ' ')"
