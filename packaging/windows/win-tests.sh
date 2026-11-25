#!/usr/bin/env bash
# Test suites on the Windows runner (port, W3, 2026-11-08), built with MSVC and run with Qt's Windows
# platform, one at a time with a time limit. Runs in Git Bash (packaging/windows/win-ci.sh) after the build.
#
#   packaging/windows/win-tests.sh <build dir> <Qt dir> <program folder> <results dir>
#
# Prints the test runner's line per suite ("<name> rc=<n> pass=<n> fail=<n> time=<s>s ..."), each
# FAIL line with the suite's name, and at the end "tests: <n> suites, <m> failed". The suites'
# output goes to <results>/tests/<name>.txt.
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=$1
QT=$2
DIST=$(cd "$3" && pwd)
OUT=$4
mkdir -p "$OUT/tests"
# the suites for Windows; more join as they are made to work here
SUITES="qb_tests qb_ui_tests qb_fixes_tests qb_w2_tests qb_w3_tests qb_w4_tests qb_w4c_tests qb_v0602_tests qb_v0601_tests qb_v0650_tests qb_v0604_tests qb_v0700_tests qb_fuzz_tests qb_v0811_tests qb_v064_tests qb_v0611_tests qb_v0603_tests qb_v0812_tests qb_v0813_tests qb_v0900_tests qb_v0901_tests qb_v0902_tests qb_v0903_tests qb_v0904_tests qb_m7_tests qb_m8_tests qb_m9_tests qb_v018_tests qb_v0212_tests qb_v0402_tests qb_v0403_tests qb_v0605_tests qb_v0642_tests qb_v0910_tests qb_v0911_tests qb_v0913_tests qb_v0920_tests qb_m10_tests qb_v0930_tests qb_v0930b_tests qb_v0930c_tests qb_v0930d_tests qb_v0930e_tests qb_v0930f_tests qb_saveas_tests qb_notesplain_tests qb_mmoverview_tests qb_v0931_tests qb_v017_tests qb_v0940a_tests qb_v0940b_tests qb_v0940c_tests qb_v0940d_tests qb_v0940e_tests"
# W4a (2026-11-11): suites written for Linux after W3, run to see where they stand on Windows; their failures are
# reported ("explore") but do not count for the result until they are made to work here and move up to SUITES
# (v0810 is left out: it needs the Linux fault shim, tests/faultshim.cpp, an LD_PRELOAD library)
# W4c (2026-11-12): v0650, v0604 and v0700 counted (their failures were Linux assumptions of the tests: /proc, a shell
# script, CRLF, the runner's own settings file, a file walk that hung on Windows' file order); fuzz, v0811, v064, v0611
# and v0603, which passed in every run since W4a, counted too. New suites written for Linux start here again.
# 0.90.4 (2026-11-13): the six suites of 0.81.2 to 0.90.3 and the new v0904 are counted above (the Scrivener format,
# the usability items, the theme list, backups and sync programs, the Windows requirements). The other Linux suites
# were tried once (run of f1934fe; the writer's choice: those that pass unchanged are counted, the rest stay Linux only
# and are listed). Passed unchanged and counted since: m7, m8, m9, v018, v0212, v0402, v0403, v0605, v0642.
# Not passing as they are, and why (nothing here was changed for them):
#   m6     paper sizes: the test expects A4, the runner's region gives US Letter
#   m10    the writing tools' packages are looked for under their Linux names (build/addons-dist/tool-*.qbpkg); the
#          test then ends with a crash of its own
#   m11    Media Playback as the Linux add-on (PipeWire, FFmpeg in the add-on, .so files)
#   m12    the importers' packages under their Linux names; ran into the 5 minute limit
#   m13    the menu audit: four menu places of add-ons that the run does not install; ran into the 5 minute limit
#   m14    printing into PDF files read with poppler's tools; ran into the 5 minute limit
#   v017   one mind map check (a board of its own for another document)
#   v019   not built: POSIX only (sys/resource.h; crash reports and signals as Linux has them)
#   v020   not built: POSIX only (sys/socket.h)
#   v021   the list of General's parts has "This Computer" on Windows
#   v0211  three checks that choose a font the runner does not have in that form
#   v040   the license list and the trash as Linux has them (.so names, the desktop's trash folder)
#   v0401  font designs separated through fontconfig (Linux's way; Windows lists each design as a family, see v0904)
#   v050   the split of the Linux media add-ons and the source list of Ubuntu's packages
#   v061   Speech Transcription as the Linux add-on (PulseAudio, Vulkan through Mesa, .so files)
#   v062   the first start's backup page (Windows has no standard backup folder) and a PDF read with poppler's tools
#   v0641  all 25 checks it reached passed; typing in the 1,000-row table ran into the 5 minute limit
#   v0810, audit1  not tried: they start child runs with the Linux fault library (tests/faultshim.cpp, LD_PRELOAD)
# 0.91.0 (2026-11-13): v0910 (backups straight in the backup folder, the seven toolbars and the icon bar) passed its
# first run here (run of 1614366, 32 checks) and is counted above.
# 0.91.1 (2026-11-14): v0911 (toolbars rearranged freely) passed here (run of 447b9cd, 42 checks; its first run had one
# failure of the test itself: a settings file read as NativeFormat, which is the registry on Windows) and is counted above.
# Windows 0.91.3 (2026-11-15): v0913 (add-on keys and certificates, the official add-ons' tools) is counted from its
# first run here (one full run less; a failure fails the run all the same); it runs python3 and openssl as the release
# workflow does (python3 is given the runner's Python below).
# Windows 0.92.0 (2026-11-20): v0920 (the fixed mind map, the shown lock, the check for updates with its Windows
# hand-over to the setup program, the backups' names with an ID, checked copies) is counted from its first run here.
# v017 and m10 are looked at again ("explore": reported, not counted): v017's mind map check, because 0.92.0 changed
# the mind map, and m10's own crash, which was never examined (run of f7bb321: the suite used a writing tool that was
# not loaded, because it looked for the tools' packages under their Linux names; it takes the Windows packages now).
# v0913 leaves out its check of an Android package on Windows (its stand-in for readelf is a shell script).
# Windows 0.93.0: m10 counted (its 77 checks passed in every run since 2026-11-20); the suites of Linux 0.93.0 counted from
# their first run here, as v0913 and v0920 were: v0930 (its child runs with the fault library are Linux only: built
# without QB_FAULT_SHIM), v0930b to v0930f, saveas, notesplain, mmoverview. The Python suites of 0.93.0 (deps: the Linux
# dependency packs; verlines: the version lines in CMakeLists.txt; secwatch) check the repository's files and run on
# Linux only. v017 stays here until item 19 (the mind map's background shapes in the MSVC build) is settled.
# Windows 0.93.1: v0931 counted from its first run here (the generic font names, with checks of its own for Windows: to-do
# item 56; the bounded plain texts; System Information's new lines); the Python suite addonentry runs on Linux only.
# Windows 0.94.0: v017 counted again (to-do item 32: the inspector follows the binder's choice; fixed in shared code in
# 0.94.0); the suites of Linux 0.94.0 counted from their first run here: v0940a to v0940e (v0940b leaves out the
# AppImage's move and the application menu of the Linux desktop: Windows has neither).
EXPLORE=""
# D2 (Windows 0.91.3): QB_WIN_SUITES (packaging/windows/win-ci.sh, from "[suites: ...]" in a commit message) runs only
# the suites it names, as the runner prints them (qb_tests, ui, v0913 ...); a name of no suite fails the run
if [ -n "${QB_WIN_SUITES:-}" ]; then
    keep=""
    keepx=""
    unknown=""
    for want in $QB_WIN_SUITES; do
        found=0
        for s in $SUITES $EXPLORE; do
            n=${s#qb_}; n=${n%_tests}; [ "$n" = "tests" ] && n=qb_tests
            if [ "$n" = "$want" ]; then
                found=1
                case " $EXPLORE " in *" $s "*) keepx="$keepx $s" ;; *) keep="$keep $s" ;; esac
            fi
        done
        [ $found -eq 1 ] || unknown="$unknown $want"
    done
    SUITES=$(echo $keep)
    EXPLORE=$(echo $keepx)
    echo "suites of this run: $SUITES $EXPLORE"
    if [ -n "$unknown" ]; then
        echo "tests: unknown suite names:$unknown"
        exit 1
    fi
fi
# the restore of a sparse 2.3 GB file (v0810) and the 4.3 GB backup (v020): not on the runner's disk
export QB_SKIP_BIG_BACKUP=1

cmake --build "$BUILD" --target $SUITES $EXPLORE -- -k 0 > "$OUT/tests-build.log" 2>&1
brc=$?
grep -E ': (fatal )?error [A-Z]+[0-9]+|error LNK|FAILED:' "$OUT/tests-build.log" | sort -u > "$OUT/tests-errors.txt"
echo "tests build: rc=$brc, failed files $(grep -c '^FAILED:' "$OUT/tests-errors.txt")"

# the fonts the Linux test runs see (DejaVu, Carlito, Caladea, FreeSerif, from Ubuntu 24.04's packages), installed
# for the whole runner: many checks name these families, and Windows would put another font in their place.
# QB_TEST_FONTS_ZIP names their archive (the workflows' job "test-fonts" makes it; since Windows 0.92.0 that is the
# only way: no archive is kept on a branch any more).
fonts="$OUT/test-fonts"
rm -rf "$fonts" && mkdir -p "$fonts"
have_fonts=0
if [ -n "${QB_TEST_FONTS_ZIP:-}" ]; then
    cp "$(cygpath -u "$QB_TEST_FONTS_ZIP")" "$fonts/f.zip" 2> /dev/null && have_fonts=1
fi
if [ $have_fonts -eq 1 ]; then
    ( cd "$fonts" && 7z x -y f.zip > /dev/null )
    cat > "$fonts/install.ps1" <<'PS'
param([string]$dir)
Add-Type -Name F -Namespace W -MemberDefinition '[DllImport("gdi32.dll", CharSet=CharSet.Unicode)] public static extern int AddFontResourceW(string f);'
$reg = 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts'
$n = 0
Get-ChildItem -Path $dir -Filter *.ttf | ForEach-Object {
    $to = Join-Path $env:WINDIR ('Fonts\' + $_.Name)
    Copy-Item $_.FullName $to -Force
    $face = (New-Object -ComObject Shell.Application).NameSpace($_.DirectoryName).ParseName($_.Name).ExtendedProperty('System.Title')
    if (-not $face) { $face = $_.BaseName }
    New-ItemProperty -Path $reg -Name ($face + ' (TrueType)') -Value $_.Name -PropertyType String -Force | Out-Null
    [void][W.F]::AddFontResourceW($to)
    $n++
}
"fonts installed: $n"
PS
    powershell -NoProfile -ExecutionPolicy Bypass -File "$(cygpath -w "$fonts/install.ps1")" "$(cygpath -w "$fonts/test-fonts")"
else
    echo "test fonts: not fetched (checks that name DejaVu, Carlito ... will see other fonts)"
fi
rm -rf "$fonts"

# the runner's screen is 1024 x 768, smaller than the windows of many checks (1400 x 950): an
# attempt at 1920 x 1080 (ChangeDisplaySettings); the line says what the screen is afterwards
cat > "$OUT/screen.ps1" <<'PS'
Add-Type -TypeDefinition @'
using System; using System.Runtime.InteropServices;
public static class D {
  [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
  public struct DEVMODE {
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string dmDeviceName;
    public short dmSpecVersion, dmDriverVersion, dmSize, dmDriverExtra; public int dmFields;
    public int dmPositionX, dmPositionY, dmDisplayOrientation, dmDisplayFixedOutput;
    public short dmColor, dmDuplex, dmYResolution, dmTTOption, dmCollate;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string dmFormName;
    public short dmLogPixels; public int dmBitsPerPel, dmPelsWidth, dmPelsHeight, dmDisplayFlags, dmDisplayFrequency;
    public int dmICMMethod, dmICMIntent, dmMediaType, dmDitherType, dmReserved1, dmReserved2, dmPanningWidth, dmPanningHeight;
  }
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern bool EnumDisplaySettings(string d, int m, ref DEVMODE dm);
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern int ChangeDisplaySettings(ref DEVMODE dm, int f);
}
'@
$dm = New-Object D+DEVMODE
$dm.dmSize = [System.Runtime.InteropServices.Marshal]::SizeOf($dm)
[void][D]::EnumDisplaySettings($null, -1, [ref]$dm)
$before = "$($dm.dmPelsWidth) x $($dm.dmPelsHeight)"
$dm.dmPelsWidth = 1920; $dm.dmPelsHeight = 1080; $dm.dmFields = 0x180000
$rc = [D]::ChangeDisplaySettings([ref]$dm, 0)
[void][D]::EnumDisplaySettings($null, -1, [ref]$dm)
"screen: $before -> $($dm.dmPelsWidth) x $($dm.dmPelsHeight) (ChangeDisplaySettings $rc)"
PS
powershell -NoProfile -ExecutionPolicy Bypass -File "$(cygpath -w "$OUT/screen.ps1")" 2>&1 | tr -d '\r' | tail -3
rm -f "$OUT/screen.ps1"

# the dictionaries beside the test programs (the program finds them beside itself)
rm -rf "$BUILD/tests/dictionaries" && cp -r "$DIST/dictionaries" "$BUILD/tests/dictionaries"
export PATH="$(cygpath -u "$QT")/bin:$PATH"
export QB_TEST_DICTIONARIES="$(cygpath -w "$DIST/dictionaries")"
export QT_QPA_PLATFORM=windows
# Windows 0.91.3: the suites that run Python call it "python3" as on Linux; the runner's Python has only python.exe,
# so a copy named python3.exe goes beside it (a Windows program looks a name up with ".exe" only)
if ! python3 -c 'import sys; sys.exit(0 if sys.version_info[0] == 3 else 1)' > /dev/null 2>&1; then
    pydir=$(python -c 'import os, sys; print(os.path.dirname(sys.executable))' 2> /dev/null | tr -d '\r')
    [ -n "$pydir" ] && pydir=$(cygpath -u "$pydir")
    if [ -n "$pydir" ] && [ -f "$pydir/python.exe" ]; then
        cp "$pydir/python.exe" "$pydir/python3.exe" && echo "python3: a copy of $pydir/python.exe"
    fi
fi
echo "python3 for the suites: $(python3 --version 2>&1 | tr -d '\r'); openssl: $(openssl version 2>&1 | tr -d '\r') ($(command -v openssl))"
# W4b: as the Linux runner does, no first start questions (the portable program's backup folder question is one)
export QB_SKIP_FIRST_START=1
# W4c (2026-11-12): the Windows add-on "Speech Transcription" of this run for qb_w4c_tests, and whisper.cpp's smallest
# model for its real recognition (ggml-tiny.bin, 77,691,713 bytes, SHA-256 checked; downloaded 2026-11-12 from
# whisper.cpp's model files on Hugging Face, cached by the workflow in .models)
export QB_TEST_ADDONS_DIR="$(cygpath -m "$OUT/addons")"   # v0650's scratch pad (W4c)
if [ -f "$OUT/addons/speech-transcription-windows.qbpkg" ]; then
    export QB_TEST_SPEECH_PACKAGE="$(cygpath -w "$OUT/addons/speech-transcription-windows.qbpkg")"
fi
TINY="$ROOT/.models/ggml-tiny.bin"
TINY_SHA=be07e048e1e599ad46341c8d2a135645097a538221678b7acdd1b1919c6e1b21
if ! echo "$TINY_SHA  $TINY" | sha256sum -c --quiet > /dev/null 2>&1; then
    mkdir -p "$ROOT/.models"
    curl -sSL --retry 3 -o "$TINY" https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.bin || true
fi
if echo "$TINY_SHA  $TINY" | sha256sum -c --quiet > /dev/null 2>&1; then
    export QB_SPEECH_MODEL_FILE="$(cygpath -w "$TINY")"
    echo "speech model for the tests: ggml-tiny.bin"
else
    echo "speech model for the tests: not fetched (the real recognition checks of w4c are not run)"
fi
n=0
bad=0
xbad=0
for s in $SUITES $EXPLORE; do
    explore=0
    case " $EXPLORE " in *" $s "*) explore=1 ;; esac
    name=${s#qb_}
    name=${name%_tests}
    [ "$name" = "tests" ] && name=qb_tests
    exe="$BUILD/tests/$s.exe"
    n=$((n + 1))
    if [ ! -f "$exe" ]; then
        echo "$name rc=- not built$([ $explore -eq 1 ] && echo ' (explore)')"
        if [ $explore -eq 1 ]; then xbad=$((xbad + 1)); else bad=$((bad + 1)); fi
        continue
    fi
    shots="$OUT/tests/shots-$name"
    mkdir -p "$shots"
    # W4b: a data folder of its own for each suite (the portable program would use build\tests\data for all, and the
    # program it starts, build\data); the child processes get it too
    rm -rf "$BUILD/tests/data-$name"
    export QB_DATA_DIR="$(cygpath -m "$(cd "$BUILD/tests" && pwd)/data-$name")" # absolute: the suites run in build\tests
    # W4c: the speech add-on's suite in a data folder named with letters outside ASCII (as C:\Users\Jürgen\...)
    if [ "$name" = w4c ]; then
        rm -rf "$BUILD/tests/data-$name Prüfung Ä ß"
        export QB_DATA_DIR="$QB_DATA_DIR Prüfung Ä ß"
    fi
    start=$(date +%s)
    limit=900
    [ $explore -eq 1 ] && limit=300 # W4a: an explore suite that hangs (v0700 in run 5) costs 5 minutes, not 15
    ( cd "$BUILD/tests" && timeout $limit "./$s.exe" "$(cygpath -w "$shots")" > "$OUT/tests/$name.txt" 2>&1 )
    rc=$?
    t=$(( $(date +%s) - start ))
    out=$(tr -d '\r' < "$OUT/tests/$name.txt")
    pass=$(grep -c '^PASS' <<< "$out")
    fail=$(grep -c '^FAIL' <<< "$out")
    last=$(grep -E '[0-9]+ failure\(s\)' <<< "$out" | tail -1)
    echo "$name rc=$rc pass=$pass fail=$fail time=${t}s ${last:-no end line}$([ $explore -eq 1 ] && echo ' (explore)')"
    grep '^FAIL' <<< "$out" | head -40 | sed "s/^/  $name /"
    if [ $rc -ne 0 ]; then
        if [ $explore -eq 1 ]; then xbad=$((xbad + 1)); else bad=$((bad + 1)); fi
    fi
    # screenshots stay small: only the first 30 of a suite
    ls "$shots" 2>/dev/null | tail -n +31 | while read -r f; do rm -f "$shots/$f"; done
done
( cd "$OUT/tests" && 7z a -tzip ../test-output.zip . > /dev/null ) && rm -rf "$OUT/tests"
echo "tests: $n suites, $bad failed (explore: $xbad of $(echo $EXPLORE | wc -w) with failures)"
# the build of an explore suite may fail without failing the run: only the counted suites' files decide
cbad=0
for s in $SUITES; do [ -f "$BUILD/tests/$s.exe" ] || cbad=1; done
[ $bad -eq 0 ] && [ $cbad -eq 0 ]
