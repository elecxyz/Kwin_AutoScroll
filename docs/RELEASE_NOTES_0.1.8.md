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
KWin 6.7.5. SteamOS 3.8, Kubuntu 26.04, and Fedora 44 builds retain their
existing KWin targets. Activation modes and saved settings are unchanged.

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
| Fedora KDE 44 / Kinoite | `6.7.5-1.fc44` | `kwin-autoscroll-0.1.8-1.fc44.x86_64.rpm` |

### Fedora KDE 44

```sh
sudo dnf install ./kwin-autoscroll-0.1.8-1.fc44.x86_64.rpm
```

Remove it with `sudo dnf remove kwin-autoscroll`.

Fedora Kinoite can layer the same RPM, then reboot:

```sh
sudo rpm-ostree install ./kwin-autoscroll-0.1.8-1.fc44.x86_64.rpm
systemctl reboot
```

### Important compatibility note

KWin effects are tied to the KWin patch version they were built against.
Check your installed KWin package version and download the matching asset.
Source compatibility spans these versions; each binary still requires its
matching KWin patch version. Packages and source outputs are collected in
`dist/0.1.8/`, alongside SHA-256 checksums.

Earlier verified 0.1.6 assets for SteamOS KWin `6.4.3-1.13` and Kubuntu KWin
`4:6.6.5-0ubuntu0.1` remain suitable only for systems still on those versions.

### SteamOS note

Installing on SteamOS requires temporarily disabling its read-only system
protection. SteamOS updates may remove the package or require a new build if
KWin changes. Steam Input compatibility has not been fully runtime-tested.
