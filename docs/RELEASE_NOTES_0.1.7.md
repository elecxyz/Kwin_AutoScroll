<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# KWin AutoScroll 0.1.7

This release refreshes the packages for current distribution KWin versions.
The **Click or hold (Windows-style)** behavior introduced in 0.1.6 is unchanged:
a middle click released inside the configured dead zone toggles AutoScroll.
Keep the button held and move outside the dead zone to scroll only while
holding; releasing the button then stops AutoScroll. Once a gesture leaves the
dead zone, returning to the anchor does not turn it back into a toggle.

The existing **Click to toggle** and **Hold to scroll** options remain
available, and existing settings retain their behavior.

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
| CachyOS | `6.7.5-1.1` | `kwin-autoscroll-0.1.7-1-cachyos-kwin6.7.5-1.1-x86_64.pkg.tar.zst` |
| SteamOS 3.8 | `6.4.3-1.16` | `kwin-autoscroll-0.1.7-1-steamos-kwin6.4.3-1.16-x86_64.pkg.tar.zst` |
| Kubuntu 26.04 | `4:6.6.6-0ubuntu0.1` | `kwin-autoscroll_0.1.7-1-kubuntu26.04-kwin6.6.6_amd64.deb` |
| Fedora KDE 44 / Kinoite | `6.7.5-1.fc44` | `kwin-autoscroll-0.1.7-1.fc44.x86_64.rpm` |

### Fedora KDE 44

```sh
sudo dnf install ./kwin-autoscroll-0.1.7-1.fc44.x86_64.rpm
```

Remove it with `sudo dnf remove kwin-autoscroll`.

Fedora Kinoite can layer the same RPM, then reboot:

```sh
sudo rpm-ostree install ./kwin-autoscroll-0.1.7-1.fc44.x86_64.rpm
systemctl reboot
```

### Important compatibility note

KWin effects are tied to the KWin patch version they were built against.
Check your installed KWin package version and download the matching asset.
SteamOS uses KWin 6.4.3 here; Fedora and CachyOS use KWin 6.7.5.

Earlier verified 0.1.6 assets for SteamOS KWin `6.4.3-1.13` and Kubuntu KWin
`4:6.6.5-0ubuntu0.1` remain suitable only for systems still on those versions.

### SteamOS note

Installing on SteamOS requires temporarily disabling its read-only system
protection. SteamOS updates may remove the package or require a new build if
KWin changes. Steam Input compatibility has not been fully runtime-tested.
