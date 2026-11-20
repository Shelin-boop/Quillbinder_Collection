#!/usr/bin/env bash
# Android 0.92.0: the preview test of the tablet's windows (tests/mobile_fit_tests.cpp) at every size it is meant for,
# on a desktop (Linux, the build with -DQB_MOBILE_PREVIEW=ON). The virtual screen of each run is the tablet's usable
# area; 72 dpi is Android's measure for the style.
#
#   packaging/android/run-tablet-fit.sh <build dir> [<pictures dir>]
#
# Sizes: a 12.7" tablet at scale 2 (1472x920) and Android's bars taken off (1472x836), 1920x1080, 1280x720, 1178x660,
# 960x540, two upright tablets (800x1200, 736x1100) and the room left with the on-screen keyboard up (1472x420).
# One line per size; the exit code is the number of sizes with failures.
set -u
B=${1:?build dir}
SHOTS=${2:-}
T="$B/tests/qb_mobile_fit_tests"
[ -x "$T" ] || { echo "no $T (configure with -DQB_MOBILE_PREVIEW=ON and build qb_mobile_fit_tests)"; exit 2; }
bad=0
for size in 1472x920 1472x836 1920x1080 1280x720 1178x660 960x540 800x1200 736x1100 1472x420; do
  log="$B/tablet-fit-$size.log"
  QT_FONT_DPI=72 timeout 900 xvfb-run -a -s "-screen 0 ${size}x24" "$T" ${SHOTS:+"$SHOTS"} > "$log" 2>&1
  rc=$?
  fails=$(grep -c '^FAIL' "$log")
  checks=$(grep -cE '^(PASS|FAIL)' "$log")
  windows=$(sed -n 's/.*(\([0-9]*\) windows opened by.*/\1/p' "$log")
  echo "$size: rc=$rc, $checks checks, $fails failed, ${windows:-?} windows"
  grep -E '^FAIL|^      BAD' "$log" | head -12
  [ "$rc" -ne 0 ] && bad=$((bad + 1))
done
exit $bad
