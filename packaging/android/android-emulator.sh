#!/usr/bin/env bash
# Start check of the Android trial build on an emulated tablet (port step A0, 2026-11-14), run by
# .github/workflows/android.yml after the APK for the tablet (arm64) was built. The emulator's system images are
# x86_64, so the program is built a second time for that ABI (packaging/android/android-ci.sh); the check then
# installs that APK on an emulated tablet with Android 14, starts it, takes pictures of the screen and keeps the
# system's log. It answers only "does the program start, and what does it show"; nobody types or touches there
# but this script. Like the build script it never stops at the first failure; its files in the results folder
# start with "emu-".
#
#   packaging/android/android-emulator.sh <results dir>
#
# Needs what android-ci.sh needs, plus /dev/kvm (GitHub's Linux runners have it) and the SDK's command line tools.
set -u
OUT=${1:-results}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
SUM="$OUT/emu-summary.txt"
: > "$SUM"
say() { echo "$*" | tee -a "$SUM"; }
t0=$(date +%s)
mins() { echo "$(( ($(date +%s) - t0) / 60 )) min"; }
SDK=${ANDROID_SDK_ROOT:?}
API=${QB_EMU_API:-34}                       # Android 14, as the tablet the trial is for
IMAGE="system-images;android-$API;default;x86_64"
PKG=io.github.shelin_boop.quillbinder.trial
# the tablet's screen (2944 x 1840 at about 2x) in its own units: the same layout with a quarter of the pixels
W=${QB_EMU_WIDTH:-1472}; H=${QB_EMU_HEIGHT:-920}; DPI=${QB_EMU_DPI:-160}

# ---- the program for the emulator's ABI
bash packaging/android/android-ci.sh "$OUT/x86_64" x86_64 > "$OUT/emu-build-steps.txt" 2>&1
for f in summary.txt errors.txt errors-context.txt; do
  [ -s "$OUT/x86_64/$f" ] && cp "$OUT/x86_64/$f" "$OUT/emu-build-$f"
done
[ -f "$OUT/x86_64/build.log" ] && tail -c 400000 "$OUT/x86_64/build.log" > "$OUT/emu-build.log"
[ -f "$OUT/x86_64/apk.log" ] && tail -c 200000 "$OUT/x86_64/apk.log" > "$OUT/emu-apk.log"
APK=$(ls "$OUT"/x86_64/*.apk 2>/dev/null | head -1)
bres=$(grep '^result:' "$OUT/x86_64/summary.txt" 2>/dev/null | tail -1)
say "x86_64 build: ${bres:-no result} ($(mins))"
if [ -z "$APK" ]; then
  rm -rf "$OUT/x86_64"
  say "emulator: not started (no APK for x86_64)"
  exit 0
fi
cp "$APK" /tmp/quillbinder-emu.apk
rm -rf "$OUT/x86_64"
# room on the disk: the emulator refuses to start with little free space (run 4: "not enough disk space to run
# avd"). The build folders are no longer needed, nor are the NDKs and other large tools GitHub's runner carries.
if [ -n "${GITHUB_ACTIONS:-}" ]; then
  rm -rf build-android-arm64-v8a build-android-x86_64
  for d in "$SDK"/ndk/*; do sudo rm -rf "$d"; done
  sudo rm -rf /usr/share/dotnet /opt/ghc /usr/local/share/boost /opt/hostedtoolcache/CodeQL 2>/dev/null
fi
say "disk: $(df -h --output=avail / | tail -1 | tr -d ' ') free"

# ---- the emulator
SDKM=$(ls "$SDK"/cmdline-tools/*/bin/sdkmanager 2>/dev/null | sort -V | tail -1)
AVDM=$(dirname "${SDKM:-/none/x}")/avdmanager
if [ ! -x "$SDKM" ]; then say "emulator: no sdkmanager in $SDK/cmdline-tools"; exit 0; fi
echo 'KERNEL=="kvm", GROUP="kvm", MODE="0666", OPTIONS+="static_node=kvm"' | sudo tee /etc/udev/rules.d/99-kvm4all.rules > /dev/null
sudo udevadm control --reload-rules
sudo udevadm trigger --name-match=kvm
say "kvm: $(ls -l /dev/kvm 2>&1)"
yes 2>/dev/null | "$SDKM" --install "emulator" "platform-tools" "$IMAGE" > "$OUT/emu-sdk.log" 2>&1
say "system image: rc=$?, $IMAGE ($(mins))"
export ANDROID_AVD_HOME="$HOME/.android/avd"
mkdir -p "$ANDROID_AVD_HOME"
echo no | "$AVDM" create avd --force -n qbtablet -k "$IMAGE" >> "$OUT/emu-sdk.log" 2>&1
CFG="$ANDROID_AVD_HOME/qbtablet.avd/config.ini"
if [ ! -f "$CFG" ]; then
  tail -20 "$OUT/emu-sdk.log" >> "$SUM"
  say "emulator: the virtual device could not be made"
  exit 0
fi
sed -i -E '/^(hw\.lcd\.(width|height|density)|hw\.initialOrientation|hw\.keyboard|hw\.ramSize|disk\.dataPartition\.size|hw\.gpu\.(enabled|mode)|showDeviceFrame)[ ]*=/d' "$CFG"
{
  echo "hw.lcd.width=$W"; echo "hw.lcd.height=$H"; echo "hw.lcd.density=$DPI"
  echo "hw.initialOrientation=landscape"; echo "hw.keyboard=yes"; echo "hw.ramSize=3072"; echo "disk.dataPartition.size=2G"
  echo "hw.gpu.enabled=yes"; echo "hw.gpu.mode=swiftshader_indirect"; echo "showDeviceFrame=no"
} >> "$CFG"
ADB="$SDK/platform-tools/adb"
"$ADB" start-server > /dev/null 2>&1
"$SDK/emulator/emulator" -avd qbtablet -no-window -no-audio -no-boot-anim -no-snapshot -gpu swiftshader_indirect \
  -no-metrics > "$OUT/emu-emulator.log" 2>&1 &
EMU=$!
booted=0
for _ in $(seq 1 150); do # up to 10 minutes
  if [ "$("$ADB" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" = 1 ]; then booted=1; break; fi
  kill -0 $EMU 2>/dev/null || break
  sleep 4
done
say "emulator: booted=$booted ($(mins))"
if [ $booted -ne 1 ]; then
  tail -30 "$OUT/emu-emulator.log" >> "$SUM"
  kill $EMU 2>/dev/null
  say "emulator: did not start"
  exit 0
fi
sleep 10
"$ADB" shell input keyevent 82 > /dev/null 2>&1          # unlock
"$ADB" shell settings put global window_animation_scale 0 > /dev/null 2>&1
"$ADB" shell settings put global transition_animation_scale 0 > /dev/null 2>&1
"$ADB" shell settings put global animator_duration_scale 0 > /dev/null 2>&1
say "device: Android $("$ADB" shell getprop ro.build.version.release | tr -d '\r') (API $("$ADB" shell getprop ro.build.version.sdk | tr -d '\r')), screen $("$ADB" shell wm size | tr -d '\r' | sed 's/.*: //'), density $("$ADB" shell wm density | tr -d '\r' | sed 's/.*: //')"

shot() { "$ADB" exec-out screencap -p > "$OUT/emu-$1.png" 2>/dev/null; [ -s "$OUT/emu-$1.png" ] || rm -f "$OUT/emu-$1.png"; }
alive() { "$ADB" shell pidof "$PKG" 2>/dev/null | tr -d '\r'; }
front() { "$ADB" shell dumpsys activity activities 2>/dev/null | grep -E 'topResumedActivity|mResumedActivity' | head -1 | tr -d '\r' | sed 's/^ *//'; }

# ---- install and start
"$ADB" install -r /tmp/quillbinder-emu.apk > "$OUT/emu-install.txt" 2>&1
say "install: rc=$?, $(tail -1 "$OUT/emu-install.txt" | tr -d '\r')"
"$ADB" logcat -c
"$ADB" shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 > "$OUT/emu-start.txt" 2>&1
say "start: rc=$? ($(mins))"
pid=""
for s in 1 2; do # two looks, 20 s apart (the emulator is slow)
  sleep 20
  pid=$(alive)
  shot "start-$s"
  say "after $((s * 20)) s: process ${pid:-gone}; in front: $(front)"
done
# One step of the check: what it does, then whether the program still runs. A program that ended is started again,
# so that the steps after it are tried too.
died=0
step() { # <name> <text for the summary>
  local name=$1; shift
  sleep 2
  if [ -n "$(alive)" ]; then
    say "  step $name ($*): runs"
  else
    died=$((died + 1))
    say "  step $name ($*): THE PROGRAM ENDED; started again"
    "$ADB" shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 > /dev/null 2>&1
    sleep 25
    "$ADB" shell input keyevent 66 > /dev/null 2>&1; sleep 5
  fi
}
key() { "$ADB" shell input keyevent "$1" > /dev/null 2>&1; }
tap() { "$ADB" shell input tap "$1" "$2" > /dev/null 2>&1; }
MX=${QB_EMU_MENU_X:-18}; MY=${QB_EMU_MENU_Y:-36} # the first menu of the menu bar (below Android's status bar)
TX=${QB_EMU_TEXT_X:-700}; TY=${QB_EMU_TEXT_Y:-330}  # a place in the text, right of the menus and above the keyboard
# closes an open menu with a tap beside it, then puts the cursor back into the text (Esc would leave the menu bar
# waiting for letters: run 7 typed into the menus that way)
# The first tap is beside the page, right of where any menu of the menu bar reaches (run 17: a tap at the text's
# place chose "System Information" from the open Help menu, and that window stayed open for all later steps).
back_to_text() { tap 1050 300; sleep 2; tap "$TX" "$TY"; sleep 2; }
if [ -n "$pid" ]; then
  say "steps:"
  # ---- A: the tablet's shared folders (android/MobileFiles.cpp). The first start asks for Android's "All files
  # access": a tap on the window's first button opens Android's own page. The switch on that page is set from here
  # (appops, as a finger on it would), and Back returns to the program, whose window sees the access and closes by
  # itself. From then on projects and backups go to Documents/Quillbinder.
  # (Run 19 pressed Enter instead: the window, shown right after the main window, did not get the key, and the Back
  # key then put the whole app away, with the window still open, for all later steps.)
  shot "a1-question"
  tap 725 510; sleep 6; shot "a2-android-page"
  fr=$(front)
  say "  A: in front after a tap on the window's first button: $fr"
  "$ADB" shell appops set --uid "$PKG" MANAGE_EXTERNAL_STORAGE allow > /dev/null 2>&1
  sleep 1
  case "$fr" in *"$PKG/"*) say "  A: ANDROID'S PAGE DID NOT COME; the access was given from here all the same" ;; *) key 4 ;; esac
  sleep 7; shot "a3-back"
  if ! "$ADB" logcat -d 2>/dev/null | grep -q 'files (the first question)'; then
    say "  A: THE WINDOW DID NOT CLOSE BY ITSELF; its button (now OK) is tapped"
    tap 725 422; sleep 4; shot "a4-ok"
  fi
  step "A" "the question about the shared folders, allowed on Android's page (appops: $("$ADB" shell appops get "$PKG" MANAGE_EXTERNAL_STORAGE 2>/dev/null | tr -d '\r' | head -1)); in front: $(front)"
  # the first start's windows: Enter presses the default button of each
  for s in 1 2; do key 66; sleep 8; step "enter-$s" "Enter in the first start's windows"; done
  shot "s0-project"
  "$ADB" shell ls -la /sdcard/Documents/Quillbinder > "$OUT/emu-shared-folder.txt" 2>&1
  say "  the tablet's Documents/Quillbinder after the first start: $(tr -d '\r' < "$OUT/emu-shared-folder.txt" | awk 'NR > 1 && $NF != "." && $NF != ".." { for (i = 8; i <= NF; i++) printf "%s ", $i; printf "| " }' | cut -c1-240)"

  # ---- B: the toolbar rows at other screen sizes (Android gives the running app's window the new size; the
  # emulated tablet counts one unit per pixel, so these are the program's own units). Before the first tap into the
  # text, so that the keyboard is not in play yet.
  screen() { # <name> <width>x<height> | reset
    if [ "$2" = reset ]; then "$ADB" shell wm size reset; else "$ADB" shell wm size "$2"; fi > /dev/null 2>&1
    sleep 7
    shot "$1"
    step "B $1" "the screen set to $2 ($("$ADB" shell wm size | tr -d '\r' | tr '\n' ' '))"
  }
  screen bars-1280x800 1280x800   # an 11-inch tablet
  screen bars-800x1280 800x1280   # the same held upright
  screen bars-back reset

  # ---- F: fingers, with raw touch events (as root; the emulator's plain system image allows it). `input swipe` is
  # of no use here: on the emulator its first move comes about 0.4 s after the finger came down (run 13: the program
  # took it for "rested 428 ms, then moved", rightly), and it cannot do two fingers at all.
  command -v compare > /dev/null 2>&1 || sudo apt-get install -y -q imagemagick > /dev/null 2>&1
  changed() { # <picture> <picture>: how many pixels of the text's part differ
    command -v compare > /dev/null 2>&1 || { echo "?"; return; }
    compare -metric AE -fuzz 8% "$OUT/emu-$1.png[520x200+450+200]" "$OUT/emu-$2.png[520x200+450+200]" null: 2>&1 | awk '{print int($1)}'
  }
  shown() { "$ADB" shell dumpsys input_method 2>/dev/null | grep -o 'mInputShown=[a-z]*' | head -1 | tr -d '\r'; }
  "$ADB" root > /dev/null 2>&1; sleep 4; "$ADB" wait-for-device > /dev/null 2>&1; sleep 2
  "$ADB" shell getevent -pl > "$OUT/emu-input-devices.txt" 2>&1
  # (the emulator has eleven touch devices, one per display it could have: the first display's is ..._touch_1)
  tdev=$(tr -d '\r' < "$OUT/emu-input-devices.txt" | awk '/^add device/ {d=$NF} /name:/ && /"virtio_input_multi_touch_1"/ {print d; exit}')
  tmax() { tr -d '\r' < "$OUT/emu-input-devices.txt" | awk -v d="$tdev" -v k="$1" '/^add device/ {cur=$NF}
    cur==d && $0 ~ k { for (i = 1; i <= NF; i++) if ($i == "max") { v=$(i+1); gsub(",", "", v); print v; exit } }'; }
  xmax=$(tmax ABS_MT_POSITION_X); ymax=$(tmax ABS_MT_POSITION_Y)
  slots=$(tr -d '\r' < "$OUT/emu-input-devices.txt" | awk -v d="$tdev" '/^add device/ {cur=$NF} cur==d && /ABS_MT_SLOT/ {print "yes"; exit}')
  px() { echo $(( $1 * xmax / W )); }
  py() { echo $(( $1 * ymax / H )); }
  raw_head() { echo "D=$tdev"; echo 's() { sendevent $D $1 $2 $3; }'; }
  # one finger from <x1> <y1> to <x2> <y2> in 8 steps, one right after the other; "rest": it stands still before it goes up
  raw_swipe() {
    {
      raw_head
      echo "s 3 47 0; s 3 57 $((700 + RANDOM % 200)); s 3 53 $(px $1); s 3 54 $(py $2); s 3 58 60; s 3 48 6; s 1 330 1; s 0 0 0"
      for k in 1 2 3 4 5 6 7 8; do
        echo "s 3 53 $(px $(($1 + ($3 - $1) * k / 8))); s 3 54 $(py $(($2 + ($4 - $2) * k / 8))); s 0 0 0"
      done
      [ "${5:-}" = rest ] && echo "sleep 0.4"
      echo "s 3 57 -1; s 1 330 0; s 0 0 0"
    } > /tmp/emu-raw.sh
    "$ADB" shell sh < /tmp/emu-raw.sh > /dev/null 2>&1
  }
  # one finger put down at <x> <y>, resting there for <seconds> (0.8 if not given), then up
  raw_hold() {
    {
      raw_head
      echo "s 3 47 0; s 3 57 $((700 + RANDOM % 200)); s 3 53 $(px $1); s 3 54 $(py $2); s 3 58 60; s 3 48 6; s 1 330 1; s 0 0 0"
      echo "sleep ${3:-0.8}"
      echo "s 3 57 -1; s 1 330 0; s 0 0 0"
    } > /tmp/emu-raw.sh
    "$ADB" shell sh < /tmp/emu-raw.sh > /dev/null 2>&1
  }
  # two fingers on a line around <x> <y>, from <a> px apart to <b> px apart
  raw_pinch() {
    {
      raw_head
      echo "s 3 47 0; s 3 57 $((300 + RANDOM % 100)); s 3 53 $(px $(($1 - $3 / 2))); s 3 54 $(py $2); s 3 58 60; s 3 48 6"
      echo "s 3 47 1; s 3 57 $((500 + RANDOM % 100)); s 3 53 $(px $(($1 + $3 / 2))); s 3 54 $(py $2); s 3 58 60; s 3 48 6"
      echo "s 1 330 1; s 0 0 0"
      for k in 1 2 3 4 5 6; do
        d=$(( $3 + ($4 - $3) * k / 6 ))
        echo "s 3 47 0; s 3 53 $(px $(($1 - d / 2))); s 3 47 1; s 3 53 $(px $(($1 + d / 2))); s 0 0 0"
      done
      echo "s 3 47 0; s 3 57 -1; s 3 47 1; s 3 57 -1; s 1 330 0; s 0 0 0"
    } > /tmp/emu-raw.sh
    "$ADB" shell sh < /tmp/emu-raw.sh > /dev/null 2>&1
  }
  if [ -n "$tdev" ] && [ -n "$xmax" ] && [ -n "$ymax" ] && [ "$slots" = yes ]; then
    # two fingers apart on the text: it grows, and is then longer than its part of the window
    raw_pinch 700 330 100 220; sleep 3; shot "f1-zoomed"
    step "F1" "two fingers moved apart on the text: $(changed bars-back f1-zoomed) pixels of the text's part changed, keyboard $(shown)"
    raw_swipe 700 600 700 380; sleep 4; shot "f2-swiped-up"
    step "F2" "a finger swiped 220 px upward over the text and went up moving: $(changed f1-zoomed f2-swiped-up) pixels changed, keyboard $(shown)"
    raw_swipe 700 380 700 600 rest; sleep 3; shot "f3-swiped-down"
    step "F3" "swiped 220 px back down, resting before it went up: $(changed f2-swiped-up f3-swiped-down) pixels changed, keyboard $(shown)"
    raw_swipe 600 400 820 404; sleep 3; shot "f4-swiped-sideways"
    step "F4" "a swipe sideways over the text: $(changed f3-swiped-down f4-swiped-sideways) pixels changed"
    raw_swipe 130 380 130 200; sleep 3; shot "f5-binder"
    step "F5" "a swipe over the binder (its list is short: nothing to scroll)"
    raw_pinch 700 330 220 100; sleep 3; shot "f6-zoomed-back"
    step "F6" "two fingers moved together again: $(changed bars-back f6-zoomed-back) pixels differ from before the fingers, keyboard $(shown)"
    # ---- D: the lines between panes, moved by a finger put beside them (places as on the 1472 x 920 screen: the
    # line between binder and text at x 259 to 262, between text and inspector at about 1156, between the
    # inspector's synopsis and its notes at about y 352). Each is moved and moved back.
    moved() { "$ADB" logcat -d 2>/dev/null | grep -c 'line between two panes moved'; }
    m0=$(moved)
    raw_swipe 268 300 402 304 rest; sleep 3; shot "d1-binder-wider"
    step "D1" "a finger put 6 px beside the binder's edge and moved 134 px to the right: lines moved so far $(( $(moved) - m0 ))"
    raw_swipe 404 300 270 304 rest; sleep 3; shot "d2-binder-back"
    step "D2" "and back: lines moved so far $(( $(moved) - m0 ))"
    raw_swipe 1150 500 1040 504 rest; sleep 3; shot "d3-inspector-wider"
    step "D3" "the inspector's edge moved 110 px to the left: lines moved so far $(( $(moved) - m0 ))"
    raw_swipe 1042 500 1152 504 rest; sleep 3
    step "D4" "and back: lines moved so far $(( $(moved) - m0 ))"
    raw_swipe 1300 360 1300 430 rest; sleep 3; shot "d5-synopsis-taller"
    step "D5" "the line under the inspector's synopsis moved 70 px down: lines moved so far $(( $(moved) - m0 ))"
    raw_swipe 1300 432 1300 362 rest; sleep 3; shot "d6-back"
    step "D6" "and back: lines moved so far $(( $(moved) - m0 ))"
  else
    say "  steps F and D (fingers): not tried: touch device '${tdev:-none}', max ${xmax:-?} x ${ymax:-?}, slots ${slots:-no} (emu-input-devices.txt)"
  fi

  # S1: a menu by finger right after the start
  tap "$MX" "$MY"; sleep 2; shot "s1-menu"; step "S1" "tap on the File menu after the start"
  back_to_text; shot "s1-text"
  # S2: typed text, as a hardware keyboard sends it
  "$ADB" shell input text 'Android%s' > /dev/null 2>&1
  sleep 3; shot "s2-typed"; step "S2" "a typed word"
  # S3: the menu while the on-screen keyboard is up
  for s in 1 2; do
    tap "$MX" "$MY"; sleep 2; [ $s = 1 ] && shot "s3-menu"
    step "S3.$s" "the File menu while the on-screen keyboard is up"
    back_to_text
  done
  # S7: every menu of the menu bar once while the on-screen keyboard is up: ten windows drawn for the first time
  # while Android's keyboard program is at work (places as on the 1472 x 920 screen)
  for item in "Edit:54" "Insert:96" "View:141" "Navigate:197" "Project:259" "Documents:330" "Format:400" "Window:460" "Help:511"; do
    tap "${item##*:}" "$MY"; sleep 2
    [ "${item%%:*}" = Format ] && shot "s7-format-menu"
    step "S7 ${item%%:*}" "the ${item%%:*} menu while the on-screen keyboard is up"
    back_to_text
  done
  # S4: a long press on a word (a selection with handles), then the menu
  "$ADB" shell input swipe "$TX" 240 "$TX" 240 900 > /dev/null 2>&1
  sleep 3; shot "s4-selected"
  step "S4-select" "long press on a word"
  tap "$MX" "$MY"; sleep 2; shot "s4-menu"
  step "S4" "long press on a word, then the File menu"
  back_to_text
  # H: the handles of a text (android/MobileHandles.cpp). The pictures show them (Android draws them above the app).
  if [ -n "${tdev:-}" ]; then
    tap "$TX" "$TY"; sleep 2; tap "$TX" "$TY"; sleep 0.6; shot "h1-tapped"
    step "H1" "a tap into the text (the drop under the cursor may show for three seconds)"
    "$ADB" shell input text 'x' > /dev/null 2>&1; sleep 1.5; shot "h2-typed"
    step "H2" "a typed letter (the drop is gone)"
    "$ADB" shell input swipe "$TX" 240 "$TX" 240 900 > /dev/null 2>&1; sleep 2; shot "h3-selected"
    step "H3" "long press on a word (the selection's two handles)"
    raw_swipe 700 385 700 215; sleep 3; shot "h4-selection-scrolled-out"
    step "H4" "the text swiped 170 px upward: the selected word is out of its pane (no handles over the toolbars): $(changed h3-selected h4-selection-scrolled-out) pixels of the text's part changed"
    raw_swipe 700 215 700 385 rest; sleep 3; shot "h5-selection-back"
    step "H5" "and swiped back (the handles again)"
    back_to_text
    # M: the menus a right click opens, by finger (android/MobileTouch.cpp). A finger that rests on the binder's
    # first document and goes up; then a word selected by a long press and a tap on it.
    menus() { "$ADB" logcat -d 2>/dev/null | grep -cE 'fingers: (rested [0-9]+ ms without moving|tap on selected text)'; }
    n0=$(menus)
    # (these menus are wide: they are closed with a tap on the status bar, far from any of their entries. Run 21:
    # the tap beside the page, which closes the menu bar's menus, chose "Comment" from the text's menu.)
    close_menu() { tap 900 850; sleep 2; tap "$TX" "$TY"; sleep 2; }
    raw_hold 68 123 0.8; sleep 2; shot "m1-binder-menu"
    step "M1" "a finger rested 0.8 s on the binder's first document and went up: menus asked for so far $(( $(menus) - n0 ))"
    close_menu
    "$ADB" shell input swipe "$TX" 240 "$TX" 240 900 > /dev/null 2>&1; sleep 2
    tap 692 240; sleep 2; shot "m2-text-menu"
    step "M2" "a word selected by a long press, then a tap on it: menus asked for so far $(( $(menus) - n0 ))"
    close_menu
  fi
  # K: a backup (Ctrl+Shift+B is File > Back Up > Back Up Now): it must arrive in Documents/Quillbinder/Backups.
  # The program tells where it saved it (a window with OK: run 21 left it open, and it took every later touch).
  "$ADB" shell input keycombination 113 59 30 > /dev/null 2>&1; sleep 8; shot "k1-backup"
  "$ADB" shell ls -la /sdcard/Documents/Quillbinder/Backups > "$OUT/emu-backups.txt" 2>&1
  step "K" "Back Up Now: zip files in the tablet's Documents/Quillbinder/Backups: $(grep -c '\.zip' "$OUT/emu-backups.txt")"
  tap 927 236; sleep 2; key 111; sleep 1; shot "k2-closed"
  # P: put away by Android. A word is typed and Home pressed at once (before the program's own autosave, which
  # waits for a pause of two seconds): the word must be in the project's files on the shared storage.
  tap "$TX" "$TY"; sleep 2
  "$ADB" shell "input text zanzibarquay; input keyevent 3" > /dev/null 2>&1
  sleep 6; shot "p1-home"
  "$ADB" shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 > /dev/null 2>&1; sleep 7; shot "p0-typed"
  found=$("$ADB" shell "grep -rl zanzibarquay /sdcard/Documents/Quillbinder 2>/dev/null | head -2" | tr -d '\r' | tr '\n' ' ')
  say "  P: a typed word, then Home at once: the word is on the shared storage in: ${found:-NO FILE}"
  "$ADB" shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 > /dev/null 2>&1; sleep 7; shot "p2-back"
  step "P" "Home, then the app again: in front: $(front)"
  # S6: windows opened from the File menu while the text has the keyboard. With Qt 6.12.0 these ENDED THE PROGRAM
  # (runs 8 to 14): the menu closes, the text gets the focus back, Android's input method starts anew and calls
  # into the program, and Qt aborts when the new window is first drawn during such a call ("Failed to acquire
  # deadlock protector for QAndroidPlatformOpenGLWindow::eglSurface()"). The same hit the menus themselves (S3,
  # S4, S7) once the input method was active. Android 0.92.0's guard (android/MobileWindows.cpp) made each window's
  # surface ahead of its first picture; Qt 6.12.1 makes the surface without that lock (Android 0.93.0: the guard is
  # gone), and these steps check that. File > Save As (0.93.0) asks for the format first on a tablet.
  # Places in the menu as on the 1472 x 920 screen, read from the run's picture of the File menu (0.93.0: New Project
  # at 62, then 27 px a row, a line between groups; the places of 0.92.0 were a row too high and opened New Project,
  # Compare and Transfer and Compile): Open Project 89, Save As 199, Compile 363, Print Settings 392.
  for item in "Open Project:89" "Save As:199" "Compile:363" "Print Settings:392"; do
    tap "$MX" "$MY"; sleep 2; tap 100 "${item##*:}"; sleep 4
    shot "s6-$(echo "${item%%:*}" | tr 'A-Z ' 'a-z-')"
    step "S6 ${item%%:*}" "File > ${item%%:*} with the cursor in the text"
    key 111; sleep 2
    back_to_text
  done
  # O: File > Show Project in File Manager (android/MobileOpen.cpp): the tablet's Files app at the project's folder.
  # The last step (0.93.0: 475, below Print Current Document and a line; 452 was Print Current Document).
  tap "$MX" "$MY"; sleep 2; shot "o0-file-menu"; tap 100 475; sleep 5; shot "o1-files-app"
  fr=$(front)
  say "  O: File > Show Project in File Manager: in front: $fr"
  case "$fr" in *"$PKG/"*) key 111; sleep 1 ;; *) key 4; sleep 3 ;; esac
  step "O" "back from the Files app: in front: $(front)"
  shot "end"
  say "the program ended $died times in these steps; now: process $(alive); in front: $(front)"
fi
# what the program wrote on the device (the app's own folders; "run-as" does not work on a release build)
"$ADB" shell ls -R "/sdcard/Android/data/$PKG" > "$OUT/emu-files.txt" 2>&1
"$ADB" shell ls -R /sdcard/Documents/Quillbinder 2>&1 | head -150 >> "$OUT/emu-shared-folder.txt"

# ---- the log: the program's own lines, Qt's, and crashes
"$ADB" logcat -d -v time > /tmp/emu-logcat-all.txt 2>/dev/null
grep -nE "fingers:|files: |files \(|tablet: |open: |windows: (surface|no context)|handles: the cursor|eglMakeCurrent|QRhi|deadlock protector|locked by|quillbinder|Quillbinder|libQt6| Qt |QtActivity|qt\.|AndroidRuntime|FATAL|DEBUG   |tombstone|signal [0-9]+|backtrace|UnsatisfiedLink|ClassNotFound|ActivityManager.*($PKG|Killing|died)" \
  /tmp/emu-logcat-all.txt | cut -c1-500 | tail -n 1500 > "$OUT/emu-logcat.txt"
# what the program took each touch for (android/MobileTouch.cpp logs a line per touch)
{ echo "touches, as the program took them:"; grep 'fingers:' /tmp/emu-logcat-all.txt | sed -E 's/^[0-9-]+ ([0-9:.]+) .*fingers: /  \1 /' | head -90; } >> "$SUM"
# the start on a tablet: the shared folders, the dictionaries, the plug-ins, the save when the app is put away
{ echo "folders, dictionaries, plug-ins, saving (android/MobileFiles.cpp, MobileStart.cpp):"
  grep -E '(files|tablet|open): |files \(' /tmp/emu-logcat-all.txt | grep -vE 'fingers:' | sed -E 's/^[0-9-]+ ([0-9:.]+) .*(files[: (]|tablet: |open: )/  \1 \2/' | cut -c1-330 | head -45; } >> "$SUM"
# new windows: Qt 6.12.1 makes their surfaces itself (Android 0.93.0; Qt 6.12.0 ended the program there)
{
  echo "new windows: Qt's lock fault of 6.12.0 (\"deadlock protector for QAndroidPlatformOpenGLWindow::eglSurface()\", which ended the program; must be 0): $(grep -c 'deadlock protector for .QAndroidPlatformOpenGLWindow' /tmp/emu-logcat-all.txt)"
  # (Qt's other uses of that lock only warn and go on, e.g. its event loop while the keyboard program calls: listed)
  echo "Qt's warnings about that lock elsewhere (harmless): $(grep 'deadlock protector' /tmp/emu-logcat-all.txt | grep -vc 'QAndroidPlatformOpenGLWindow')"
  grep 'deadlock protector' /tmp/emu-logcat-all.txt | grep -v 'QAndroidPlatformOpenGLWindow' | sed -E 's/^[0-9-]+ ([0-9:.]+) [^:]*: /  \1 /' | cut -c1-160 | head -4
  echo "drawing contexts that did not fit a surface (must be 0): $(grep -cE 'eglMakeCurrent failed|Failed to create QRhi|Could not create the egl surface' /tmp/emu-logcat-all.txt)"
  # (Qt draws once more while Android takes the app's surfaces away; seen in run 19 at the Home key, harmless there)
  echo "pictures Qt could not draw (\"Failed to make context current\"; expected only when the app is put away): $(grep -c 'Failed to make context current' /tmp/emu-logcat-all.txt)"
  grep -E 'Failed to make context current|put away by Android' /tmp/emu-logcat-all.txt | sed -E 's/^[0-9-]+ ([0-9:.]+) .*: /  \1 /' | cut -c1-120 | head -8
  echo "the cursor's handle, as android/MobileHandles.cpp allowed it:"
  grep 'handles: the cursor' /tmp/emu-logcat-all.txt | sed -E 's/^[0-9-]+ ([0-9:.]+) .*handles: /  \1 /' | tail -16
} >> "$SUM"
crashes=$(grep -cE 'FATAL EXCEPTION|Fatal signal|\*\*\* \*\*\* \*\*\*' /tmp/emu-logcat-all.txt)
say "log: $(wc -l < /tmp/emu-logcat-all.txt) lines, kept $(wc -l < "$OUT/emu-logcat.txt"); crash marks $crashes"
"$ADB" emu kill > /dev/null 2>&1
sleep 3
kill $EMU 2>/dev/null

grep -E 'Abort message|Fatal signal' /tmp/emu-logcat-all.txt | sed -E 's/^[0-9-]+ [0-9:.]+ //' | sort | uniq -c | cut -c1-220 >> "$SUM"
if [ -n "$(ls "$OUT"/emu-start-*.png 2>/dev/null)" ] && [ -n "$pid" ] && [ "$crashes" = 0 ]; then
  say "emulator result: started, ran through all steps (pictures: $(ls "$OUT"/emu-*.png | wc -l))"
elif [ -n "$pid" ]; then
  say "emulator result: started, ended $died times in the steps (crash marks $crashes)"
else
  say "emulator result: did not start (crash marks $crashes)"
fi
exit 0
