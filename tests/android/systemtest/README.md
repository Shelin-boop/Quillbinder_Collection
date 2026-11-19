# The system test for Android: Android's own copy of the cases

Copied on 2026-11-16 from Linux's system test (`tests/crashsafe/*.cpp`, `*.h` and `tests/crashsafe_tests.cpp`, as on
`main` 6c23dfd, Linux 0.91.3) at the writer's word: a copy of all test cases, so that the originals for Linux are not
touched, and only the copy is used for the Android tests. Android work changes only this copy; Linux's
cases stay as they are, and a case added to Linux later does not come here by itself.

- `st_main.cpp` is the copy of `tests/crashsafe_tests.cpp`.
- The app: `tests/android/CMakeLists.txt` (built with `-DQB_ANDROID_SYSTEM_TEST=ON`); its runs on emulated tablets:
  `packaging/android/android-system-test.sh` and `.github/workflows/android-system-test.yml`.
- On the desktop, for trying cases before they run on a tablet: `-DQB_MOBILE_PREVIEW=ON`, target
  `qb_android_st_preview` (`tests/CMakeLists.txt`); what needs Android itself is inside `Q_OS_ANDROID`.
- Not copied: `faultinject.cpp`, `keys.py`, `sender.py` (Linux tools: a fault library loaded with LD_PRELOAD, the
  key senders of the typing gate).

## What differs from Linux (2026-11-16)

- **Taken out (21 cases)**: what needs the fault library (C-34, C-35, C-36, C-38, C-43, S-D06, S-N01), a second copy of
  the real program (X-01 to X-04, X-07 to X-12) and the real key sender of Linux (S-W17). The typing gate G7 types
  through a thread of its own here (`cs_common.cpp`, `KeySender`). Skipped for now: `packaging/android/system-test-skip.txt`.
- **New (4 cases, `cs_android.cpp`)**: A-F01 the tablet's folders, A-A01 the translation add-on (installed, loaded at
  the next start, translating with a model taken "From Folder..."), A-K01 killed while typing, A-H01 Home, then killed.
- **Sub-runs**: a tablet app cannot start a second copy of itself. `subCase()` writes a line into `subcases.txt`; the
  script runs the case again as `<case>@<variant>` (`QB_ST_KEEP_DATA=1`: the app's data are kept, as after a restart).
- **Actions from outside** (the case list's "needs"): K, the script kills the app once the case wrote `kill-now`; H, it
  presses Home once the case wrote `home-now`, waits 6 s, then kills it; T, the script fetches the add-on's Android test
  package (pre-release `test-android` of the add-on's public repository, checked against its SHA256SUMS) and the model
  opus-mt-de-en and puts them under `QB_CS_SOURCE` (`addons/`, `models/`).
- **Harness**: windows are hidden before they are deleted (`dropWindow`); waiters of one kind of window answer in the
  order they were armed, and a waiter that already looked in vain ends at the next step or the next waiter of its kind;
  `ScopedWaiter` ends a waiter with the loop step that armed it (U-04, U-05, S-A07, S-T01); PDF text
  through Qt PDF instead of pdftotext; zip files through `cs_zip.cpp`; the writing tools end after each case
  (`shutdownTools`, as `main.cpp` on quitting).
- **Known program defect worked around** (Linux code, for a Linux job): the undo watcher of the main window
  (`src/ui/MainWindowUndo.cpp`) is reached during `~MainWindow` (U-05; A-A01 with the keyboard in the add-on's panel:
  the watcher's freed timer is started, found with valgrind). The copy hides windows first and gives the keyboard back
  to the text before the end.
