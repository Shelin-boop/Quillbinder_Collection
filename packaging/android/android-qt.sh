#!/usr/bin/env bash
# Qt for the Android build (port step A0, 2026-11-14): The Qt Company's own binaries of the version named in
# CMakeLists.txt (QB_QT_VERSION), for the build machine (Qt's tools: moc, rcc, androiddeployqt) and for Android,
# side by side in one folder: <base>/<version>/gcc_64, <base>/<version>/android_arm64_v8a, .../android_x86_64.
# Installed with aqtinstall 3.3.0, as for the Linux and Windows builds; it checks every archive against the
# checksums of download.qt.io. Needs download.qt.io (GitHub's runners reach it; a Claude session does not).
#
#   packaging/android/android-qt.sh <base folder> [log folder]
#
# Qt PDF: since Qt 6.8 Qt's repository keeps it apart from the other modules ("extensions"), and for Android in a
# folder named after the Qt version and the ABI (all_os/extensions/qtpdf/6120/qt6_6120_arm64_v8a/, seen in the
# repository's listings 2026-11-14). aqtinstall 3.3.0 looks in .../6120/android_arm64_v8a/ and finds nothing
# (run 1 of the Android workflow: "The packages ['qtpdf'] were not found"), so this script gives it the folder
# name before it runs, as the Windows workflow does for Qt's Windows folders.
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BASE=${1:?base folder}
LOGS=${2:-.}
mkdir -p "$BASE" "$LOGS"
V=$(sed -n 's/^set(QB_QT_VERSION *"\(.*\)")/\1/p' "$ROOT/CMakeLists.txt")
ARCHS=${QB_QT_ANDROID_ARCHS:-android_arm64_v8a android_x86_64}

python3 -m pip install -q --break-system-packages 'aqtinstall==3.3.0' 2>/dev/null || python3 -m pip install -q 'aqtinstall==3.3.0' || exit 1
python3 - <<'PY' || exit 1
import pathlib
import aqt.archives as m
p = pathlib.Path(m.__file__)
s = p.read_text(encoding="utf-8")
mark = "# Quillbinder A0: Android's extension folders"
old = '            for ext in ["qtwebengine", "qtpdf"]:\n'
new = ('            if self.target == "android":  ' + mark + '\n'
       '                arch = "qt{}_{}_{}".format(self.version.major, self._version_str(), self.arch[len("android_"):])\n'
       + old)
if mark not in s:
    assert s.count(old) == 1, "aqtinstall changed: the extensions loop was not found"
    p.write_text(s.replace(old, new, 1), encoding="utf-8")
print("aqtinstall patched:", p)
PY

python3 -m aqt list-qt all_os android --modules "$V" android_arm64_v8a > "$LOGS/qt-android-modules.txt" 2>&1 || true
if [ ! -f "$BASE/$V/gcc_64/bin/qmake" ]; then
  python3 -m aqt install-qt linux desktop "$V" linux_gcc_64 -O "$BASE" 2>&1 | tee "$LOGS/qt-install.log" || exit 1
fi
for arch in $ARCHS; do
  abi=${arch#android_}
  python3 -m aqt install-qt all_os android "$V" "$arch" -O "$BASE" -m qtpdf qtimageformats 2>&1 | tee -a "$LOGS/qt-install.log" \
    || echo "aqtinstall ended with an error for $arch (see qt-install.log)"
  ls "$BASE/$V/$arch/lib" 2>/dev/null | grep -cE "^libQt6(Pdf|PdfWidgets|Core|Widgets|PrintSupport|Svg|Network|Xml)_" | sed "s/^/$arch: Qt libraries of the program found (of 8): /"
done
rm -rf "$BASE/$V"/*/doc "$BASE/$V"/*/examples
echo "Qt $V: $(ls "$BASE/$V" | tr '\n' ' ')"
