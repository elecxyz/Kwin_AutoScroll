<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# KWin AutoScroll 0.1.8

This release adds KWin 6.8 compatibility while retaining support for older
KWin versions. The build detects the scroll-event API provided by the target
headers and supplies the new autoscroll flag when supported. Tests cover
horizontal, vertical, and stop events on each target.

CachyOS packages now target KWin 6.8.0. A new Kubuntu 26.10 package targets
KWin 6.7.5. SteamOS 3.8 and Kubuntu 26.04 builds retain their existing KWin
targets. Activation modes and saved settings are unchanged.

The **5 November 2026 Fedora packaging refresh** rebuilds Fedora 44 for KWin
6.8.2 and adds Fedora 45 with the same KWin patch version. AutoScroll remains
at 0.1.8; the Fedora RPM release increases to `2`.

<table align="center">
  <tr>
    <td valign="top">
      <img
        src="https://github.com/user-attachments/assets/e8c63462-66ae-4c92-ac35-2cb60e1c6927"
        height="520"
        alt="AutoScroll Configuration">
    </td>
    <td width="20"></td>
    <td valign="center">
      <img
        src="https://github.com/user-attachments/assets/ae190cb0-569a-423f-9320-11c864e48ec2"
        height="400"
        alt="Pointer Style Selection">
    </td>
  </tr>
</table>

## Packages

| Distribution | Required KWin package | Release asset |
|---|---|---|
| CachyOS | `6.8.0-1.1` | `kwin-autoscroll-0.1.8-1-cachyos-kwin6.8.0-1.1-x86_64.pkg.tar.zst` |
| SteamOS 3.8 | `6.4.3-1.16` | `kwin-autoscroll-0.1.8-1-steamos-kwin6.4.3-1.16-x86_64.pkg.tar.zst` |
| Kubuntu 26.04 | `4:6.6.6-0ubuntu0.1` | `kwin-autoscroll_0.1.8-1-kubuntu26.04-kwin6.6.6_amd64.deb` |
| Kubuntu 26.10 | `4:6.7.5-0ubuntu1` | `kwin-autoscroll_0.1.8-1-kubuntu26.10-kwin6.7.5_amd64.deb` |
| Fedora KDE 44 / Kinoite 44 | `6.8.2-3.fc44` | `kwin-autoscroll-0.1.8-2.fc44.x86_64.rpm` |
| Fedora KDE 45 / Kinoite 45 | `6.8.2-3.fc45` | `kwin-autoscroll-0.1.8-2.fc45.x86_64.rpm` |

### Fedora KDE 44

```sh
sudo dnf install ./kwin-autoscroll-0.1.8-2.fc44.x86_64.rpm
```

Remove it with `sudo dnf remove kwin-autoscroll`.

Fedora Kinoite 44 can layer the same RPM, then reboot:

```sh
sudo rpm-ostree install ./kwin-autoscroll-0.1.8-2.fc44.x86_64.rpm
systemctl reboot
```

### Fedora KDE 45

```sh
sudo dnf install ./kwin-autoscroll-0.1.8-2.fc45.x86_64.rpm
```

Remove it with `sudo dnf remove kwin-autoscroll`.

Fedora Kinoite 45 can layer its matching RPM, then reboot:

```sh
sudo rpm-ostree install ./kwin-autoscroll-0.1.8-2.fc45.x86_64.rpm
systemctl reboot
```

### Important compatibility note

KWin effects are tied to the KWin patch version they were built against.
Check your installed KWin package version and download the matching asset.
Source compatibility spans these versions; each binary still requires its
matching KWin patch version. The new Fedora binary and source RPMs are
collected in `dist/0.1.8-fedora-kwin6.8.2/`, alongside SHA-256 checksums.
Original release outputs remain in `dist/0.1.8/`.

Fedora 44 systems still on KWin `6.7.5-1.fc44` should keep the original
`kwin-autoscroll-0.1.8-1.fc44.x86_64.rpm` package. The new Fedora 44 and 45
packages target KWin 6.8.2 and are not suitable for KWin 6.7.5.

Earlier verified 0.1.6 assets for SteamOS KWin `6.4.3-1.13` and Kubuntu KWin
`4:6.6.5-0ubuntu0.1` remain suitable only for systems still on those versions.

### Fedora build validation

The Fedora 44 and 45 RPMs were built on 5 November 2026 from source commit
`35db39d`. Each build passed all 10 automated tests, plugin version and
library-dependency checks, and installation/removal in its matching Fedora
container. RPM lint checks passed for both binary and source packages.

The separate Fedora output folder contains both binary RPMs, both source
RPMs, and `SHA256SUMS`. Live Plasma Wayland behavior and Kinoite package
layering still require the manual checks in
[RUNTIME_TESTING.md](RUNTIME_TESTING.md).

### SteamOS note

Installing on SteamOS requires temporarily disabling its read-only system
protection. SteamOS updates may remove the package or require a new build if
KWin changes. Steam Input compatibility has not been fully runtime-tested.
