#pragma once
// Signatures of packages (0.65.0, external audit of 2026-11-10, F12).
//
// The release workflow signs the description of every package it makes (manifest.json, which lists the
// SHA-256 of every file of the package) with the project's release key, an Ed25519 key that only the
// workflow holds; the signature is the package's file manifest.sig:
//   {"algorithm": "ed25519", "key": "<key id>", "signature": "<64 bytes, Base64>", "signs": "manifest.json"}
// The program carries the public half of that key and checks the signature before anything else. A
// bundle is as trustworthy as its parts: each part carries a signature of its own.
//
// Add-on keys (0.91.3, the writer's decision of 2026-11-15: one key for program updates, one or more for official
// add-ons): an add-on may instead be signed with an add-on key whose certificate travels in its manifest.sig,
//   "certificate": {"format": 1, "role": "addons", "key": "<64 hex: the add-on key>", "name": "<who holds it>",
//                   "issued": "yyyy-mm-dd", "by": "<key id of the release key>", "signature": "<Base64>"}
// signed by the release key over certificateText(). Such a key vouches for add-ons only, never for a program
// update; the release key itself signs no add-on any more once an add-on key exists (the release workflow), but
// add-ons it signed before stay trusted.
#include <QByteArray>
#include <QList>
#include <QString>

namespace qb {
namespace signing {

enum class State {
    Unsigned,   // no manifest.sig
    Valid,      // signed with a key this program trusts
    Invalid,    // a signature that does not match (changed after signing, or damaged)
    UnknownKey, // signed with a key this program does not know (counts as not signed)
};

struct Check {
    State state = State::Unsigned;
    QString key;             // the key id the signature names
    bool releaseKey = false; // Valid, and signed with the release key itself (only such a package may be a program update)
    QString signer;          // Valid through an add-on key: the name its certificate gives
};

// Ed25519 (RFC 8032): whether signature (64 bytes) is publicKey's (32 bytes) signature of message
bool verify(const QByteArray &message, const QByteArray &signature, const QByteArray &publicKey);
// the first 16 hex digits of the SHA-256 of a public key
QString keyId(const QByteArray &publicKey);
// the keys this program trusts: the project's release key (in builds with the test switches also the key in
// QB_TEST_SIGNING_KEY, 64 hex digits)
QList<QByteArray> trustedKeys();
// manifest: the bytes of manifest.json as in the package; signature: the bytes of manifest.sig ("" when none)
Check checkManifest(const QByteArray &manifest, const QByteArray &signature);
// the text a release key signs to vouch for an add-on key (0.91.3; tools/make-package.py certify writes the same):
// "Quillbinder key certificate 1\nrole: <role>\nkey: <64 lower-case hex>\nname: <name>\nissued: <date>\n"
QByteArray certificateText(const QString &role, const QByteArray &publicKey, const QString &name, const QString &issued);

} // namespace signing
} // namespace qb
