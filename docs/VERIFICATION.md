# What has been verified

Status as of October 2026. "Verified" means observed or tested as described, not a security certification.

## Automated tests

`make test` runs seven suites (profiles, prompt parser, window, two session suites, browser automation and sign-in cache). They use synthetic connection files, a fake FreeRDP, synthetic Chromium and an in-memory Secret Service adapter; no Microsoft account, real profile, personal keyring or security key is involved. The original six-suite baseline also passed under AddressSanitizer and UndefinedBehaviorSanitizer with leak detection. Those sanitizer and static-analysis results predate the remembered-sign-in change; its checks are recorded below.

## Static and dynamic analysis

- Clang static analyzer (`scan-build`: core, unix, security, C++ and dead-code checkers) over every source file: no findings. clang-tidy findings are reviewed and dispositioned.
- Bounded fuzzing of the prompt parser and connection-file import (`tools/security/`).
- An `strace` of a live sign-in and connection (file, process and network calls only). It found Chromium keeping crash data in the user's real config directory and still contacting Google background services. Both were fixed.

## Security review so far

A threat model and per-area reviews produced a ledger of 23 findings plus four deferred lifecycle items. Each was reproduced and fixed with a failing-then-passing test, closed with evidence, or accepted with a stated reason. Fixed issues include:

- an IP-literal bypass of the hostname-only rule (`2130706433`, `0x7f.0.0.1`),
- a foreign frame substituting the portal's connection-file download,
- portal automation running in a replacement page,
- signals that could reach a recycled process ID,
- a stale-write race between two app instances,
- a FIFO that could hang the UI.

A release-gate audit of the ten design invariants found two more problems: a debug-only raw FreeRDP log reachable in release builds, and a browser process-group signalling gap. Both are fixed. All of these were AI-assisted reviews (Claude Code and Codex), with fixes re-reviewed until no findings remained. **No independent human review has happened yet.** That's the main open item; see the [review brief](SECURITY-REVIEW-BRIEF.md).

## Live checks

On the maintainer's Windows 365 Enterprise tenant, with a hardware security key:

- automatic connection-file download from the portal, sign-in, desktop,
- in-session PIN and touch,
- disconnect and reconnect,
- one wrong PIN followed by the correct one,
- cancelling during sign-in, and unplugging the key during sign-in.

Not yet checked: other tenants, Frontline and other editions, multi-monitor.

## Remembered sign-in checks (October 14, 2026)

- Application build and all seven test suites, including cookie domain, expiry, session-only/partitioned-cookie exclusion, cache bounds, cancellation, failed keyring operations and Forget racing a pending save.
- BrowserAuth checks for restoring cookies before navigation, optional cookie-call failure/timeout, cancellation while loading/snapshotting and rejecting unsafe runtime storage before accessing the keyring. Window checks cover preference/deletion feedback, active-session controls and narrow layouts.
- A separate smoke check against a real private Secret Service saved, reloaded, deleted and confirmed absence of a fictional item. Another used real Chromium CDP to restore/export a fictional Microsoft-domain persistent cookie and compare its expiry, Secure, HttpOnly and SameSite attributes. No live sign-in was attempted.
- UI preference and Forget checked in an isolated omabox desktop (Omarchy 4.0.4-1, Hyprland 0.56.2, 1920×1080, scale 1, 0xleovision theme). The disposable keyring uses an empty password: this checks integration, not encryption at rest.

Not yet checked for this feature: live Windows 365/FreeRDP/passkey reuse, tenant Conditional Access behavior, federated sign-in, or other Secret Service providers. The keyring provider controls encryption and unlocking. No credentials or real browser state were used in these checks.

## Reconnect follow-up (October 15, 2026)

The account owner reports full authentication even when reconnecting without quitting
OmaWin365. Source inspection found that the connected handler closes the last sign-in
window, which the existing retained-window code already documents as clearing Chromium
session cookies. The new opt-in path retains eligible session cookies in app-lifetime
memory and restores them on subsequent flows, while keyring encoding remains persistent
only. Known persistent authentication cookies are distinguished from other saved cookies
in the UI. This follow-up was compiled; the test results above predate it, and live
reconnect behavior after this change remains unconfirmed. Reuse across app restarts
without persistent Microsoft authentication cookies remains unresolved.
