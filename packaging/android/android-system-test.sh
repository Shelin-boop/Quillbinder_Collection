#!/usr/bin/env bash
# The system test on an emulated tablet (2026-11-14), run by .github/workflows/android-system-test.yml.
#
# The Linux system test (tests/crashsafe: crash safety and usability; tools/run-crashsafe.sh starts each case as a
# process of its own) for Android: the same test program is an app here (tests/android), and this script is what
# starts it once per case on an emulated tablet:
#   - the app's data is cleared before each case (a fresh home, as the Linux runner gives each case), the access to
#     the shared folders given anew;
#   - the arguments of the start are written into a file the app reads (tests/android/st_android.cpp);
#   - the case's folder is on the tablet's shared storage, where a writer's projects are (Download/qbst/results);
#   - a case that does not end within its time is stopped; after each case the fresh process of the Linux
#     procedure checks what it left ("--verify");
#   - one line per case goes into st-summary.txt, the cases' logs into st-logs.tar.gz.
# Never stops at the first failure.
#
#   packaging/android/android-system-test.sh build <results dir>
#       builds the test program as an APK for the emulator (x86_64): <results dir>/qb-system-test.apk
#   packaging/android/android-system-test.sh run <results dir> <apk> [shard] [shards] [cases...]
#       runs cases on an emulated tablet
#
# shard/shards: this run takes the cases whose place in the list, modulo <shards>, is <shard> (several runs side by
# side); cases named after them are the only ones run, shared among the tablets the same way.
# QB_CS_PROFILE=quick|full|smoke (quick), QB_ST_TIME_FACTOR (a
# case may take this many times its estimate, which is from the Linux machine: 4), QB_ST_MAX_MIN (the whole run's
# limit in minutes: 300; cases left over are named). "build" needs what android-ci.sh needs; "run" needs the
# Android SDK's command line tools and /dev/kvm.
set -u
MODE=${1:?build or run}
OUT=${2:-results}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
if [ "$MODE" = run ]; then
  APKIN=${3:?the APK}
  SHARD=${4:-0}
  SHARDS=${5:-1}
  shift $(( $# < 5 ? $# : 5 ))
  ONLY="$*"
  SUM="$OUT/st-summary.txt"
else
  SHARD=0; SHARDS=1; ONLY=""
  SUM="$OUT/st-build-summary.txt"
fi
: > "$SUM"
say() { echo "$*" | tee -a "$SUM"; }
t0=$(date +%s)
mins() { echo "$(( ($(date +%s) - t0) / 60 )) min"; }
SDK=${ANDROID_SDK_ROOT:?}
ABI=x86_64
API=${QB_EMU_API:-34}
IMAGE="system-images;android-$API;default;x86_64"
PKG=io.github.shelin_boop.quillbinder.systemtest
ACT="$PKG/org.qtproject.qt.android.bindings.QtActivity"
DEV=/sdcard/Download/qbst
PROFILE=${QB_CS_PROFILE:-quick}
FACTOR=${QB_ST_TIME_FACTOR:-4}
MAXMIN=${QB_ST_MAX_MIN:-300}
W=${QB_EMU_WIDTH:-1472}; H=${QB_EMU_HEIGHT:-920}; DPI=${QB_EMU_DPI:-160}

if [ "$MODE" = build ]; then
say "commit: ${GITHUB_SHA:-local}"
NDK=${ANDROID_NDK_ROOT:?}
# ---- the test program as an APK for the emulator's ABI (with the test switches, as the Linux test builds)
qtver=$(sed -n 's/^set(QB_QT_VERSION *"\(.*\)")/\1/p' CMakeLists.txt)
QT_TARGET="${QT_ANDROID_BASE:?}/$qtver/android_x86_64"
QT_HOST="$QT_ANDROID_BASE/$qtver/gcc_64"
BT="$SDK/build-tools/${QB_BUILD_TOOLS:-none}"
[ -d "$BT" ] || BT=$(ls -d "$SDK"/build-tools/* 2>/dev/null | sort -V | tail -1)
LAUNCH=""
command -v sccache > /dev/null 2>&1 && LAUNCH="-DCMAKE_C_COMPILER_LAUNCHER=sccache -DCMAKE_CXX_COMPILER_LAUNCHER=sccache"
# Hunspell and OpenSSL from their packs (D1, as packaging/android/android-ci.sh), else built; the dictionaries as
# assets: the cases meet the same program as the tablet's APK (2026-11-16: the first run had neither, and the spelling
# and web page cases failed for it)
HUN="$PWD/.hunspell/$ABI"
if ! bash packaging/android/android-deps-pack.sh get "hunspell-$ABI" "$NDK" "$HUN" > "$OUT/st-hunspell.log" 2>&1; then
  bash packaging/android/android-hunspell.sh "$NDK" "$ABI" "$HUN" >> "$OUT/st-hunspell.log" 2>&1
fi
HUNARGS=""
[ -f "$HUN/lib/libhunspell-1.7.a" ] && HUNARGS="-DQB_HUNSPELL_INCLUDE=$HUN/include -DQB_HUNSPELL_LIB=$HUN/lib/libhunspell-1.7.a"
SSL="$PWD/.openssl/$ABI"
if ! bash packaging/android/android-deps-pack.sh get "openssl-$ABI" "$NDK" "$SSL" > "$OUT/st-openssl.log" 2>&1; then
  bash packaging/android/android-openssl.sh "$NDK" "$ABI" "$SSL" >> "$OUT/st-openssl.log" 2>&1
fi
SSLARGS=""
[ -f "$SSL/lib/libssl_3.so" ] && SSLARGS="-DQB_ANDROID_OPENSSL=$SSL"
ASSETS="$PWD/.apk-assets"
rm -rf "$ASSETS"
mkdir -p "$ASSETS"
bash packaging/windows/win-dictionaries.sh "$ASSETS" > "$OUT/st-dictionaries.log" 2>&1
say "hunspell: $(tail -1 "$OUT/st-hunspell.log"); openssl: ${SSLARGS:-none}; dictionaries: $(ls "$ASSETS/dictionaries" 2>/dev/null | grep -c '\.dic$') ($(mins))"
. packaging/android/android-trial-key.sh
B="build-android-st-$ABI"
cmake -S . -B "$B" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$QT_TARGET/lib/cmake/Qt6/qt.toolchain.cmake" \
  -DQT_HOST_PATH="$QT_HOST" \
  -DANDROID_SDK_ROOT="$SDK" -DANDROID_NDK_ROOT="$NDK" \
  -DQT_ANDROID_SIGN_APK=ON \
  -DQT_ANDROID_SDK_BUILD_TOOLS_REVISION="$(basename "$BT")" \
  -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON \
  -DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384 -DCMAKE_MODULE_LINKER_FLAGS=-Wl,-z,max-page-size=16384 \
  -DQB_ANDROID_BUILD="$(( ${QB_ANDROID_BUILD:-0} % 1000 ))" \
  $LAUNCH $HUNARGS $SSLARGS -DQB_ANDROID_ASSETS="$ASSETS" \
  -DQB_BUILD_TESTS=OFF -DQB_TEST_SWITCHES=ON -DQB_ANDROID_SYSTEM_TEST=ON -DQB_BUILD_TOOLS=OFF -DQB_BUILD_TOOL_PLUGINS=ON \
  -DQB_BUILD_IMPORTER_PLUGINS=ON -DQB_BUILD_MEDIA_PLUGIN=OFF -DQB_BUILD_EXAMPLE_PLUGIN=ON \
  > "$OUT/st-configure.log" 2>&1
rc=$?
say "configure: rc=$rc ($(mins))"
if [ $rc -ne 0 ]; then
  tail -60 "$OUT/st-configure.log" > "$OUT/st-errors.txt"
  say "result: configure failed"; exit 0
fi
cmake --build "$B" --target qb_system_test -- -k 0 > "$OUT/st-build.log" 2>&1
rc=$?
grep -E ': (fatal )?error: |^ld(\.lld)?: error|: undefined (reference|symbol)|^FAILED:|ninja: (error|build stopped)' "$OUT/st-build.log" \
  | sort -u > "$OUT/st-errors.txt"
say "build: rc=$rc, error lines $(grep -c 'error' "$OUT/st-errors.txt"), failed files $(grep -c '^FAILED:' "$OUT/st-errors.txt") ($(mins))"
if [ $rc -ne 0 ]; then
  grep -nE -m 80 -A4 ': (fatal )?error: ' "$OUT/st-build.log" | cut -c1-400 > "$OUT/st-errors-context.txt"
  say "result: build failed"; exit 0
fi
cmake --build "$B" --target qb_system_test_make_apk > "$OUT/st-apk.log" 2>&1
rc=$?
APK=$(find "$B/tests/android" -name '*.apk' 2>/dev/null | head -1)
say "apk: rc=$rc, ${APK:-none} ($(mins))"
if [ $rc -ne 0 ] || [ -z "$APK" ]; then
  { grep -nE -B2 -A12 'FAILURE:|What went wrong|error:|Error:|Exception|Cannot|not found|failed' "$OUT/st-apk.log" | cut -c1-400 | head -150
    echo "---- the end of st-apk.log"; tail -40 "$OUT/st-apk.log"; } > "$OUT/st-errors.txt"
  say "result: apk failed"; exit 0
fi
cp "$APK" "$OUT/qb-system-test.apk"
say "test program: qb-system-test.apk $(stat -c %s "$OUT/qb-system-test.apk") bytes"
command -v sccache > /dev/null 2>&1 && sccache --show-stats > "$OUT/st-sccache.txt" 2>&1
say "result: ok"
exit 0
fi

# ================================================================ run
say "commit: ${GITHUB_SHA:-local}; shard $SHARD of $SHARDS; profile $PROFILE; cases named: ${ONLY:-none}"
[ -f "$APKIN" ] || { say "result: no APK ($APKIN)"; exit 0; }
cp "$APKIN" /tmp/qb-system-test.apk
# room on the disk for the emulator (as android-emulator.sh)
if [ -n "${GITHUB_ACTIONS:-}" ]; then
  for d in "$SDK"/ndk/*; do sudo rm -rf "$d"; done
  sudo rm -rf /usr/share/dotnet /opt/ghc /usr/local/share/boost /opt/hostedtoolcache/CodeQL 2>/dev/null
fi
say "disk: $(df -h --output=avail / | tail -1 | tr -d ' ') free"

# ---- the emulator
SDKM=$(ls "$SDK"/cmdline-tools/*/bin/sdkmanager 2>/dev/null | sort -V | tail -1)
AVDM=$(dirname "${SDKM:-/none/x}")/avdmanager
if [ ! -x "$SDKM" ]; then say "result: no sdkmanager in $SDK/cmdline-tools"; exit 0; fi
echo 'KERNEL=="kvm", GROUP="kvm", MODE="0666", OPTIONS+="static_node=kvm"' | sudo tee /etc/udev/rules.d/99-kvm4all.rules > /dev/null
sudo udevadm control --reload-rules
sudo udevadm trigger --name-match=kvm
yes 2>/dev/null | "$SDKM" --install "emulator" "platform-tools" "$IMAGE" > "$OUT/st-sdk.log" 2>&1
say "system image: rc=$?, $IMAGE ($(mins))"
export ANDROID_AVD_HOME="$HOME/.android/avd"
mkdir -p "$ANDROID_AVD_HOME"
echo no | "$AVDM" create avd --force -n qbtablet -k "$IMAGE" >> "$OUT/st-sdk.log" 2>&1
CFG="$ANDROID_AVD_HOME/qbtablet.avd/config.ini"
if [ ! -f "$CFG" ]; then say "result: the virtual device could not be made"; exit 0; fi
sed -i -E '/^(hw\.lcd\.(width|height|density)|hw\.initialOrientation|hw\.keyboard|hw\.ramSize|disk\.dataPartition\.size|hw\.gpu\.(enabled|mode)|showDeviceFrame)[ ]*=/d' "$CFG"
{
  echo "hw.lcd.width=$W"; echo "hw.lcd.height=$H"; echo "hw.lcd.density=$DPI"
  echo "hw.initialOrientation=landscape"; echo "hw.keyboard=yes"; echo "hw.ramSize=4096"; echo "disk.dataPartition.size=6G"
  echo "hw.gpu.enabled=yes"; echo "hw.gpu.mode=swiftshader_indirect"; echo "showDeviceFrame=no"
} >> "$CFG"
ADB="$SDK/platform-tools/adb"
"$ADB" start-server > /dev/null 2>&1
"$SDK/emulator/emulator" -avd qbtablet -no-window -no-audio -no-boot-anim -no-snapshot -gpu swiftshader_indirect \
  -no-metrics > "$OUT/st-emulator.log" 2>&1 &
EMU=$!
booted=0
for _ in $(seq 1 150); do
  if [ "$("$ADB" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" = 1 ]; then booted=1; break; fi
  kill -0 $EMU 2>/dev/null || break
  sleep 4
done
say "emulator: booted=$booted ($(mins))"
if [ $booted -ne 1 ]; then
  tail -30 "$OUT/st-emulator.log" >> "$SUM"
  kill $EMU 2>/dev/null
  say "result: the emulator did not start"; exit 0
fi
sleep 10
"$ADB" shell input keyevent 82 > /dev/null 2>&1
for s in window_animation_scale transition_animation_scale animator_duration_scale; do "$ADB" shell settings put global $s 0 > /dev/null 2>&1; done
# the screen stays on, and no screen lock (a long run)
"$ADB" shell svc power stayon true > /dev/null 2>&1
"$ADB" shell settings put system screen_off_timeout 2147483647 > /dev/null 2>&1
"$ADB" root > /dev/null 2>&1; sleep 4; "$ADB" wait-for-device > /dev/null 2>&1; sleep 2
say "device: Android $("$ADB" shell getprop ro.build.version.release | tr -d '\r') (API $("$ADB" shell getprop ro.build.version.sdk | tr -d '\r')), screen $("$ADB" shell wm size | tr -d '\r' | tr '\n' ' ')"
"$ADB" install -r /tmp/qb-system-test.apk > "$OUT/st-install.txt" 2>&1
say "install: rc=$?, $(tail -1 "$OUT/st-install.txt" | tr -d '\r')"

alive() { "$ADB" shell pidof "$PKG" 2>/dev/null | tr -d '\r'; }
fresh() { # the app as never started: no data, the access to the shared folders given
  "$ADB" shell am force-stop "$PKG" > /dev/null 2>&1
  "$ADB" shell pm clear "$PKG" > /dev/null 2>&1
  "$ADB" shell appops set --uid "$PKG" MANAGE_EXTERNAL_STORAGE allow > /dev/null 2>&1
}
# Starts the app with the test program's arguments and waits until it has ended, at most <seconds>.
# Prints "ended", "timeout" (stopped then) or "no start".
run_app() { # <seconds> <log file on the device> <arguments...>
  local limit=$1 log=$2; shift 2
  {
    echo "--env"; echo "QB_CS_LOG=$log"
    echo "--env"; echo "QB_CS_FIXTURES=$DEV/fixtures"
    echo "--env"; echo "QB_CS_PROFILE=$PROFILE"
    echo "--env"; echo "QB_CS_SOURCE=$DEV/source"
    echo "--env"; echo "QB_CS_BUILD=$DEV/build"
    echo "--env"; echo "QB_SKIP_FIRST_START=1"
    [ -n "${QB_CS_SEED:-}" ] && { echo "--env"; echo "QB_CS_SEED=$QB_CS_SEED"; }
    # a sub-run's variables come last ("NAME=" takes one away: tests/android/st_android.cpp)
    for e in "${EXTRA_ENV[@]}"; do echo "--env"; echo "$e"; done
    for a in "$@"; do echo "$a"; done
  } > /tmp/qbst-args.txt
  "$ADB" push /tmp/qbst-args.txt "$DEV/next-args.txt" > /dev/null 2>&1
  "$ADB" shell am start -W -n "$ACT" > /tmp/qbst-start.txt 2>&1
  local t=0 seen=0 acted=0
  while [ $t -lt "$limit" ]; do
    if [ -n "$(alive)" ]; then seen=1; elif [ $seen -eq 1 ] || [ $t -ge 20 ]; then break; fi
    # what a case asks of the script from outside (tests/android/systemtest/cs_android.cpp): K kills the app once the
    # case wrote "kill-now" (kill -9, as Android ends an app whose memory it needs), H presses Home once it wrote
    # "home-now", waits, then kills it (an app put away is ended later without a word)
    if [ $acted -eq 0 ] && [ -n "$WATCH_NEEDS" ] && [ $seen -eq 1 ]; then
      case "$WATCH_NEEDS" in
        *K*) if [ "$("$ADB" shell "test -f $WATCH_DIR/kill-now && echo y" 2>/dev/null | tr -d '\r')" = y ]; then
               "$ADB" shell "kill -9 \$(pidof $PKG)" > /dev/null 2>&1; acted=1
               sleep 1; [ -n "$(alive)" ] && "$ADB" shell am force-stop "$PKG" > /dev/null 2>&1 # (no root: the same end)
             fi ;;
        *H*) if [ "$("$ADB" shell "test -f $WATCH_DIR/home-now && echo y" 2>/dev/null | tr -d '\r')" = y ]; then
               "$ADB" shell input keyevent 3 > /dev/null 2>&1; sleep 6
               "$ADB" shell "kill -9 \$(pidof $PKG)" > /dev/null 2>&1; acted=1
               sleep 1; [ -n "$(alive)" ] && "$ADB" shell am force-stop "$PKG" > /dev/null 2>&1
             fi ;;
      esac
    fi
    sleep 2; t=$((t + 2))
  done
  if [ -n "$(alive)" ]; then
    "$ADB" exec-out screencap -p > "$OUT/st-shots/timeout-$(basename "$(dirname "$log")").png" 2>/dev/null
    "$ADB" shell am force-stop "$PKG" > /dev/null 2>&1
    echo timeout
  elif [ $acted -eq 1 ]; then echo killed
  elif [ $seen -eq 1 ]; then echo ended; else echo "no start"; fi
}
EXTRA_ENV=(); WATCH_NEEDS=""; WATCH_DIR=""
# The add-on Translation and Proofreading for this tablet's processor and one of its models (case A-A01, needs "T").
# Android 0.92.0: the package the workflow's build job made from addons/translation-proofreading
# (packaging/android/android-translator.sh x86_64; QB_ST_ADDON_FILE), and the model from this repository's release
# models-1. Without QB_ST_ADDON_FILE the package is fetched from QB_ST_ADDON_URL and checked against its SHA256SUMS.
# Put where the case looks (QB_CS_SOURCE: addons/, models/), once.
ADDON_BASE=${QB_ST_ADDON_URL:-}
MODEL_BASE=${QB_ST_MODEL_URL:-https://github.com/Shelin-boop/Quillbinder_Collection/releases/download/models-1}
provided=""
provide_addon() {
  [ -n "$provided" ] && return 0
  provided=tried
  local d=/tmp/qbst-addon f pkg="translation-proofreading-android-$ABI.qbpkg" got=no
  rm -rf "$d"; mkdir -p "$d/models/opus-mt-de-en"
  if [ -n "${QB_ST_ADDON_FILE:-}" ] && [ -f "$QB_ST_ADDON_FILE" ]; then
    cp "$QB_ST_ADDON_FILE" "$d/$pkg" && got=yes
  elif [ -n "$ADDON_BASE" ] && curl -fsSL --retry 2 -o "$d/$pkg" "$ADDON_BASE/$pkg" && curl -fsSL --retry 2 -o "$d/SUMS" "$ADDON_BASE/SHA256SUMS-android.txt" \
     && (cd "$d" && grep " \*\?$pkg\$" SUMS | sha256sum -c - > /dev/null 2>&1); then
    got=yes
  fi
  if [ "$got" = yes ]; then
    "$ADB" shell mkdir -p "$DEV/source/addons" "$DEV/source/models" > /dev/null 2>&1
    "$ADB" push "$d/$pkg" "$DEV/source/addons/" > /dev/null 2>&1
    for f in config.json MODEL-LICENSE.txt model.bin shared_vocabulary.json source.spm target.spm; do
      curl -fsSL --retry 2 -o "$d/models/opus-mt-de-en/opus-mt-de-en--$f" "$MODEL_BASE/opus-mt-de-en--$f" || { say "add-on: the model file $f could not be fetched"; return 1; }
    done
    "$ADB" push "$d/models/opus-mt-de-en" "$DEV/source/models/" > /dev/null 2>&1
    provided=yes
    say "add-on: $pkg and the model opus-mt-de-en put on the tablet ($(mins))"
  else
    say "add-on: $pkg is not there (QB_ST_ADDON_FILE '${QB_ST_ADDON_FILE:-}', QB_ST_ADDON_URL '$ADDON_BASE')"
  fi
}
# Runs one case, or a sub-run of it, and writes its line into the summary.
# run_case <id> <title> <time limit s> <needs> <variant or ""> <keep data 0|1> [NAME=VALUE...]
run_case() {
  local id=$1 title=$2 limit=$3 needs=$4 variant=$5 keep=$6
  shift 6
  local name=$id
  [ -n "$variant" ] && name="$id@$variant"
  EXTRA_ENV=()
  [ -n "$variant" ] && EXTRA_ENV+=("QB_CS_VARIANT=$variant")
  for e in "$@"; do EXTRA_ENV+=("$e"); done
  case "$needs" in *T*) provide_addon ;; esac
  "$ADB" logcat -c > /dev/null 2>&1
  [ "$keep" = 1 ] || fresh
  local c0 r secs v=""
  c0=$(date +%s)
  WATCH_NEEDS=$needs; WATCH_DIR="$DEV/results/$name"
  r=$(run_app "$limit" "$DEV/results/$name/case.log" --case "$id" "$DEV/results/$name")
  WATCH_NEEDS=""
  secs=$(( $(date +%s) - c0 ))
  if [ "$r" = ended ] || [ "$r" = killed ]; then
    v=$(run_app 240 "$DEV/results/$name/verify.log" --verify "$DEV/results/$name")
  fi
  EXTRA_ENV=()
  mkdir -p "$OUT/st-logs/$name"
  for f in case.log verify.log facts.json subcases.txt; do "$ADB" pull "$DEV/results/$name/$f" "$OUT/st-logs/$name/$f" > /dev/null 2>&1; done
  ran=$((ran + 1))
  local log="$OUT/st-logs/$name/case.log" vlog="$OUT/st-logs/$name/verify.log" pass fail vfail end vend
  pass=$(grep -c '^PASS' "$log" 2>/dev/null); fail=$(grep -c '^FAIL' "$log" 2>/dev/null)
  vfail=$(grep -c '^FAIL' "$vlog" 2>/dev/null)
  end=$(grep -c "^END $id" "$log" 2>/dev/null)
  vend=$(grep -c '^END verify' "$vlog" 2>/dev/null)
  local outside=0
  case "$needs" in *K*|*H*) outside=1 ;; esac
  if [ "$r" = timeout ]; then
    hung=$((hung + 1)); say "HUNG  $name  $title  (stopped after $secs s; passed ${pass:-0}, failed ${fail:-0} so far; last line: $(tail -1 "$log" 2>/dev/null | cut -c1-140))"
  elif [ $outside -eq 1 ] && [ "$r" = ended ]; then
    failed=$((failed + 1)); say "FAILED  $name  $title  (the script did not end the app: no kill-now/home-now came; passed ${pass:-0}, failed ${fail:-0}; $secs s)"
  elif [ "$r" = killed ] && [ "${vend:-0}" -eq 0 ]; then
    broke=$((broke + 1)); say "ENDED EARLY  $name  $title  (killed as planned after $secs s, but the check afterwards did not end: $v; last line: $(tail -1 "$vlog" 2>/dev/null | cut -c1-140))"
  elif [ "$r" != killed ] && [ "${end:-0}" -eq 0 ]; then
    broke=$((broke + 1))
    "$ADB" logcat -d -v time 2>/dev/null | grep -E "Fatal signal|Abort message|FATAL EXCEPTION|backtrace:|#0[0-9] pc" | head -14 | cut -c1-260 > "$OUT/st-logs/$name/crash.txt"
    say "ENDED EARLY  $name  $title  ($r after $secs s; passed ${pass:-0}, failed ${fail:-0}; last line: $(tail -1 "$log" 2>/dev/null | cut -c1-140); $(head -1 "$OUT/st-logs/$name/crash.txt" 2>/dev/null | cut -c1-120))"
  elif [ "${fail:-0}" -gt 0 ] || [ "${vfail:-0}" -gt 0 ]; then
    failed=$((failed + 1)); say "FAILED  $name  $title  (passed ${pass:-0}, failed ${fail:-0}, in the check afterwards $v: failed ${vfail:-0}; $secs s)"
  else
    clean=$((clean + 1))
    if [ "$r" = killed ]; then say "clean  $name  $title  (killed as planned; passed ${pass:-0}, the check afterwards found nothing lost; $secs s)"
    else say "clean  $name  $title  (passed ${pass:-0}; $secs s)"; fi
  fi
  # the pictures the case took are many: only those of cases that did not come out clean
  if { [ "$r" != ended ] && [ "$r" != killed ]; } || [ "${fail:-0}" -gt 0 ] || [ "${vfail:-0}" -gt 0 ] || { [ "$r" = ended ] && [ "${end:-0}" -eq 0 ]; }; then
    "$ADB" shell "cd $DEV/results/$name && ls *.png 2>/dev/null | tail -3" 2>/dev/null | tr -d '\r' | while read -r p; do
      [ -n "$p" ] && "$ADB" pull "$DEV/results/$name/$p" "$OUT/st-shots/$name-$p" > /dev/null 2>&1
    done
  fi
  "$ADB" shell rm -rf "$DEV/results/$name" > /dev/null 2>&1
}

mkdir -p "$OUT/st-shots" "$OUT/st-logs"

# ---- the files the cases read from the repository, the list of cases, the fixtures
"$ADB" shell rm -rf "$DEV" > /dev/null 2>&1
"$ADB" shell mkdir -p "$DEV/source/tests" "$DEV/build" "$DEV/results" > /dev/null 2>&1
for d in tests/import tests/media tests/fonts; do [ -d "$d" ] && "$ADB" push "$d" "$DEV/source/tests/" > /dev/null 2>&1; done
"$ADB" push docs "$DEV/source/" > /dev/null 2>&1
"$ADB" shell mkdir -p "$DEV/source/tests/crashsafe" > /dev/null 2>&1
fresh
r=$(run_app 120 "$DEV/list.txt" --list)
"$ADB" pull "$DEV/list.txt" "$OUT/st-list.txt" > /dev/null 2>&1
# Android 0.93.2 (to-do item 35): lines that are no case (the emulated graphics driver writes lines such as
# "s_glBindAttribLocation: ..." into the program's output) are counted out: a case's line is "<id> <quick> <full>
# <needs> <title>", its minutes as numbers
if [ -s "$OUT/st-list.txt" ]; then
  awk 'NF >= 5 && $2 ~ /^[0-9]+$/ && $3 ~ /^[0-9]+$/' "$OUT/st-list.txt" > "$OUT/st-list-cases.txt"
  other=$(( $(grep -c . "$OUT/st-list.txt") - $(grep -c . "$OUT/st-list-cases.txt") ))
  [ "$other" -gt 0 ] && say "the list of cases: $other lines that are no case counted out (st-list.txt keeps them)"
  cp "$OUT/st-list-cases.txt" "$OUT/st-list-run.txt"; rm -f "$OUT/st-list-cases.txt"
fi
total=$(grep -c . "$OUT/st-list-run.txt" 2>/dev/null)
say "the list of cases: $r, ${total:-0} cases ($(mins))"
if [ "${total:-0}" -lt 10 ]; then
  "$ADB" exec-out screencap -p > "$OUT/st-shots/list-failed.png" 2>/dev/null
  "$ADB" logcat -d -v time 2>/dev/null | grep -E "$PKG|AndroidRuntime|FATAL|Fatal signal|libqb|Qt" | tail -120 | cut -c1-300 > "$OUT/st-logcat-list.txt"
  cat /tmp/qbst-start.txt >> "$SUM"
  say "result: the test program did not give its list of cases"
  # (Android 0.93.2, to-do item 51: a run without cases is a failure; the workflow still uploads what there is)
  "$ADB" emu kill > /dev/null 2>&1; exit 3
fi
fresh
r=$(run_app 1500 "$DEV/fixtures.log" --fixtures "$DEV/fixtures")
"$ADB" pull "$DEV/fixtures.log" "$OUT/st-fixtures.log" > /dev/null 2>&1
say "fixtures: $r, $(tail -1 "$OUT/st-fixtures.log" 2>/dev/null | tr -d '\r') ($(mins))"

# ---- the cases
skipfile=packaging/android/system-test-skip.txt
n=0; ran=0; clean=0; failed=0; hung=0; broke=0; skipped=0; left=0
# (the list is read through its own channel: "adb shell" inside the loop would swallow the rest of it, as the first
# shakedown showed, which ran one case of six)
while read -r id quick full needs title <&3; do
  [ -n "$id" ] || continue
  # named cases (2026-11-16): only those, shared among the tablets as the whole list is
  if [ -n "$ONLY" ]; then
    case " $ONLY " in *" $id "*) ;; *) continue ;; esac
  fi
  idx=$n; n=$((n + 1))
  [ $(( idx % SHARDS )) -eq "$SHARD" ] || continue
  why=$(grep -E "^${id}[[:space:]]" "$skipfile" 2>/dev/null | head -1 | sed -E "s/^${id}[[:space:]]+//")
  if [ -n "$why" ]; then
    skipped=$((skipped + 1)); say "SKIP  $id  $title  ($why)"; continue
  fi
  if [ $(( ($(date +%s) - t0) / 60 )) -ge "$MAXMIN" ]; then
    left=$((left + 1)); say "LEFT  $id  $title  (the run's time was over)"; continue
  fi
  est=$quick; [ "$PROFILE" = full ] && est=$full
  limit=$(awk -v e="$est" -v f="$FACTOR" 'BEGIN { printf "%d", e * f + 120 }')
  run_case "$id" "$title" "$limit" "$needs" "" 0
  # the sub-runs the case asked for (subcases.txt: "<variant>" and NAME=VALUE pairs; QB_ST_KEEP_DATA=1 keeps the
  # app's data, as after a restart), each counted as a case of its own: <case>@<variant>
  if [ -s "$OUT/st-logs/$id/subcases.txt" ]; then
    while IFS=$'\t' read -r -a sub <&4; do
      [ ${#sub[@]} -gt 0 ] && [ -n "${sub[0]}" ] || continue
      keep=0
      for e in "${sub[@]:1}"; do [ "$e" = QB_ST_KEEP_DATA=1 ] && keep=1; done
      run_case "$id" "$title" "$limit" "$needs" "${sub[0]}" "$keep" "${sub[@]:1}"
    done 4< "$OUT/st-logs/$id/subcases.txt"
  fi
done 3< "$OUT/st-list-run.txt"
say "cases of this run: $ran run, $clean clean, $failed with failed checks, $hung stopped at their time limit, $broke ended early, $skipped skipped, $left left over ($(mins))"
tar -C "$OUT" -czf "$OUT/st-logs.tar.gz" st-logs 2>/dev/null
rm -rf "$OUT/st-logs"
"$ADB" emu kill > /dev/null 2>&1
sleep 3
kill $EMU 2>/dev/null
say "result: done"
exit 0
