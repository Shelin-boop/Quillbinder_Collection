QtKeychain 0.18.0, vendored (Quillbinder 0.99.0)

Source:  https://github.com/frankosterfeld/qtkeychain, tag 0.18.0 (commit e86bc3cbb50b172be61ed857803a372412b6ad85)
License: COPYING in this folder (the modified BSD license of QtKeychain); also in Help > Third-Party Licenses.
Use:     the program keeps the passphrase of the writer's key in the system's key store per device
         (src/core/crypto/KeyStore.h).

What is here: the files of the folder qtkeychain/ that the backends of Linux, Windows and Android need, unchanged
except for the note below. Left out: the Apple, Haiku and WebAssembly backends, the translations, the test programs,
QtKeychain's own CMake and qmake files (CMakeLists.txt in this folder is Quillbinder's own build file).

The one change: qtkeychain/libsecret.cpp takes the three GLib functions it calls (g_error_free, g_list_free_full,
g_object_unref) from the libsecret it loads at run time, like the libsecret functions themselves, instead of linking
GLib's GObject; every change is marked "Quillbinder" in that file. So the program gets no new shared library.

A new version: replace the files of qtkeychain/ with those of the new tag, apply the change again, and raise the
version in tools/licenses-extra.json (the security watch, tools/security-watch.json, reads it there).
