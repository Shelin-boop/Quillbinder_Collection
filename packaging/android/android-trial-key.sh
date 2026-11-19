#!/usr/bin/env bash
# The key that signs Android trial builds and the system test's app. (The public APK is signed by the release
# workflow with the project's Android key: packaging/android/release-key.txt.)
#
#   . packaging/android/android-trial-key.sh
#
# Sets what Qt's androiddeployqt reads: QT_ANDROID_KEYSTORE_PATH, _ALIAS, _STORE_PASS, _KEY_PASS.
#
# Android installs an APK over an installed one only when both are signed with the same key. A kept key is named by
# QB_ANDROID_TRIAL_KEYSTORE (a PKCS #12 file), QB_ANDROID_TRIAL_KEYSTORE_PASS and QB_ANDROID_TRIAL_KEYSTORE_ALIAS
# (default qbtrial). Without them a key is made for this run alone: such a trial build cannot be installed over the
# one before (remove that one from the tablet first; its files in Documents/Quillbinder stay).
if [ -n "${QB_ANDROID_TRIAL_KEYSTORE:-}" ] && [ -f "${QB_ANDROID_TRIAL_KEYSTORE:-}" ]; then
  export QT_ANDROID_KEYSTORE_PATH="$QB_ANDROID_TRIAL_KEYSTORE"
  export QT_ANDROID_KEYSTORE_ALIAS="${QB_ANDROID_TRIAL_KEYSTORE_ALIAS:-qbtrial}"
  export QT_ANDROID_KEYSTORE_STORE_PASS="${QB_ANDROID_TRIAL_KEYSTORE_PASS:-}"
  export QT_ANDROID_KEYSTORE_KEY_PASS="$QT_ANDROID_KEYSTORE_STORE_PASS"
else
  qb_keytool=keytool
  [ -x "${JAVA_HOME:-}/bin/keytool" ] && qb_keytool="$JAVA_HOME/bin/keytool"
  qb_keydir=$(mktemp -d)
  export QT_ANDROID_KEYSTORE_PATH="$qb_keydir/trial-run.p12"
  export QT_ANDROID_KEYSTORE_ALIAS=qbtrial
  QT_ANDROID_KEYSTORE_STORE_PASS=$(head -c 24 /dev/urandom | od -An -tx1 | tr -d ' \n')
  export QT_ANDROID_KEYSTORE_STORE_PASS
  export QT_ANDROID_KEYSTORE_KEY_PASS="$QT_ANDROID_KEYSTORE_STORE_PASS"
  "$qb_keytool" -genkeypair -keystore "$QT_ANDROID_KEYSTORE_PATH" -storetype PKCS12 -alias qbtrial -keyalg RSA \
    -keysize 2048 -validity 30 -dname "CN=Quillbinder trial build" \
    -storepass "$QT_ANDROID_KEYSTORE_STORE_PASS" > /dev/null 2>&1 \
    || echo "android-trial-key: keytool could not make a key for this run" >&2
fi
