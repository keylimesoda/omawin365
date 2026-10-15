# OMAWIN365

![OMAWIN365](docs/banner.png)

**Pre-release. Looking for reviewers.** OMAWIN365 is a native Qt launcher for Windows 365 Cloud PCs on Omarchy/Arch. It signs in to Microsoft in a private Chromium window and opens your Cloud PC with FreeRDP in a separate window. Importing a connection never connects automatically.

**Why it exists: your security key works *inside* the Cloud PC.** Signing in to Windows 365 with a FIDO2 security key works in supported browsers, Omarchy included. But in my testing the browser-based client couldn't redirect a local key into the remote session, so sign-ins *inside* the Cloud PC (Microsoft 365, admin portals, websites) can't use it. OMAWIN365 uses FreeRDP's WebAuthn redirection: when something in the session asks for your key, a native PIN prompt appears on your Omarchy desktop, and you touch your local key. No raw USB passthrough is involved.

It works end to end with Windows 365 Enterprise on the maintainer's tenant, including:

- downloading the connection file from the portal,
- Microsoft sign-in with a hardware security key,
- in-session security-key sign-ins (native PIN prompt, then touch),
- reconnect, cancel and wrong-PIN recovery.

Other tenants and editions are untested. It has had extensive AI-assisted review but **no independent human security review yet**. Please [help review it](#help-review-it) before trusting it with a work account.

Not affiliated with, endorsed by or supported by Microsoft or Omarchy. Microsoft, Windows, Windows 365 and the redrawn Windows 365 mark are Microsoft trademarks, used here only for identification. Provided **as-is, without warranty**, under the [MIT license](LICENSE); see [third-party notices](THIRD_PARTY_NOTICES.md).

## Remember Microsoft sign-in

Enable **Remember Microsoft sign-in on this device** to retain Microsoft's persistent
sign-in cookies in your desktop's Secret Service keyring (for example GNOME Keyring
or KWallet with Secret Service enabled). This is off by default. If Microsoft offers
**Stay signed in**, accept it to allow a persistent session. Your organization's
Conditional Access policy, cookie expiration and revocation still apply; Microsoft
can require your passkey again. Federated identity providers may require their own
sign-in because their cookies are not saved.

While remembering is enabled, eligible Microsoft session cookies are also kept in
application memory after successful sign-in and restored for subsequent connections.
This keeps closing the sign-in window from discarding the current app session.
These session cookies are never written to the keyring and disappear when OmaWin365
exits or you choose Forget. Reuse after restarting the app still requires Microsoft
to issue persistent authentication cookies; the local checkbox cannot enable that.

**Forget Microsoft sign-in**, or turning remembering off, deletes the saved item and
closes the private sign-in browser. These controls are available while disconnected.
This only clears OMAWIN365's local sign-in state; it does not revoke Microsoft sessions
or sign you out of other applications. A deletion failure is shown so you can unlock
the keyring and retry. If the keyring is missing or locked and cannot be opened, sign-in
continues normally without a plaintext cache fallback.

The text below the checkbox reports whether sign-in was saved securely, remembered
only until the app closes, or could not be remembered. A successful keyring write
containing only other Microsoft cookies is not presented as saved authentication.
Microsoft still decides whether to accept restored sign-in state.
The optional `--diagnose-auth` log also records fixed cache outcome labels, without
cookie values, names, URLs or account identifiers.

FreeRDP still obtains fresh connection tokens using its existing OAuth flow. OMAWIN365
saves only unexpired, secure, persistent cookies for `login.microsoftonline.com`;
session-only cookies, passwords, PINs, passkeys, OAuth tokens and entire browser profiles are
not saved to the keyring. The live Chromium profile, cache and temporary files remain in private tmpfs
storage. Keyring protection and unlock behavior depend on your Secret Service provider;
applications running as your user may be able to access an unlocked keyring.

## Build

On Omarchy/Arch with Qt 6.11 (`qt6-base`, `qt6-wayland`, `qt6-svg`), a C++20 compiler, `libx11`, `libsecret` and `fontconfig`. Chromium and XWayland are needed at run time.

```sh
make -j4
make test   # needs an X11/XWayland DISPLAY; see docs/TESTING.md
./.build/omawin365
```

The app needs **FreeRDP 3.32.2 built with `WITH_SSO_MIB=OFF`**. Arch's stock `freerdp` reached 3.32.2 on 2026-10-12, but Omarchy's package mirrors still had 3.31.1 on 2026-10-13, which the app rejects. Until they catch up, the recipe in [`packaging/freerdp`](packaging/freerdp) builds a private copy under `/usr/lib/omawin365/freerdp` without touching the distro package; see [packaging](docs/PACKAGING.md).

**Why 3.32.2:** Microsoft's gateways use wildcard certificates (`*.wvd.microsoft.com`). FreeRDP 3.32.1 and older, when built without uriparser, discard wildcard names, so valid gateways look like a name mismatch. OMAWIN365 never accepts a certificate prompt, so every connection stopped. FreeRDP 3.32.2 fixes this ([FreeRDP#13653](https://github.com/FreeRDP/FreeRDP/pull/13653)), and the private copy builds it unmodified. Once Omarchy's mirrors carry 3.32.2, the package dependency will move to stock `freerdp`, and the private copy goes away.

## Help review it

The [security review brief](docs/SECURITY-REVIEW-BRIEF.md) explains what the app does, where the risk is and the five questions we most want answered. It takes about 10 minutes to read. Human reviews and AI-assisted reviews are both welcome; please read [contributing](CONTRIBUTING.md) first. Report anything exploitable privately: see [SECURITY.md](SECURITY.md).

## More

- [Architecture and boundaries](docs/BOUNDARIES.md)
- [Tests](docs/TESTING.md)
- [What has been verified](docs/VERIFICATION.md)
- [Packaging](docs/PACKAGING.md)

Built with AI coding agents (Claude Code and Codex), directed and tested by the maintainer. Never include credentials, PINs, callback URLs, tokens or real connection files in an issue.
