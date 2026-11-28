# Live Sync: who hosts (0.99.0, work package 6)

Files: `src/core/collab/{Membership,Authority,Election,Handover,Drafts}.*`. Design: sections 6.3 to 6.6 and 12 of the Live Sync
design; the suite is `ls6` (`docs/dev-notes/test-suites.md`).

- **Signatures** are `DeviceIdentity::sign/verify` (ECDSA P-256; the Ed25519 probe failed, see the notes of 0.99.0). A proof, a vote
  and a membership are signed over compact JSON (Qt writes object keys sorted, so the bytes are the same on every system).
  A member's public key is inside the membership, so a device can verify a host it never paired with; the id must equal the hash
  of the key.
- **Write before send**: every state change goes to `livesync/authority.json` (`safesave::writeFile`) before the message that depends
  on it. The old host stores RETIRED together with its proof and the lock table (`hoLocks`: a restarted old host must resend the
  same proof, whose digest covers the locks) before `HANDOVER_COMMIT`. A vote is stored before it is answered.
- **Epochs** only grow: a failed election is repeated with a higher epoch (a vote given to a candidate that never won cannot block
  the project); a target that is PREPARED promises its epoch (`highest`) even after it goes back to FOLLOWER, so no election can
  open the same epoch.
- **Chain**: every device keeps the whole chain; steps identical to stored ones are not verified again. Verification is against the
  current membership, so a proof signed by a device that has since been removed does not verify (accepted limit).
- **Split**: a host in the minority keeps hosting (decision). Two devices: the other takes over alone. A host that comes back after a
  crash is the host of its epoch until it hears the newer chain (peers send their chain on connect).
- **Reunion** (`Authority::adopt`): backup (backend), then drafts (`makeSplitDrafts`, id from project, fork, document and author, so a
  crash in the middle repeats safely), then the record is written, then hosting stops. If the drafts cannot be written nothing changes.
  Clients turn the commits of their ledger (`CommitLedger`: commits this device authored that the host confirmed) after the fork into
  drafts; a host reads its log (`authoredFromLog`). The service must feed the ledger from `ClientSession::eventApplied` (own commits)
  or `HostSession::committed`.
- **Not in this package**: the service's use of `HostingBackend`, the status window, the dialogs of the drafts, `PRESENCE` (the service
  calls `setPeerInSync`).
