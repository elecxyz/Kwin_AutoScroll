# KWin AutoScroll

Kwin AutoScroll attempts to recreate the auto scrolling functionality of Windows plus a few extra nice-to-haves.

Middle-click in an application, move the pointer away from the anchor, and the
distance and direction control the scrolling speed. Move back into the dead
zone to pause, or click again to stop.

<p align="center">
  <img
    src="https://github.com/user-attachments/assets/e8c63462-66ae-4c92-ac35-2cb60e1c6927"
    width="560"
    align="top"
    alt="Auto Scroll configuration"
  />
  <img
    src="https://github.com/user-attachments/assets/ecff29ac-7647-445e-8ff1-37c6e1a763a0"
    width="227"
    align="top"
    alt="Pointer style menu"
  />
</p>

[auto_scroll_demo.webm](https://github.com/user-attachments/assets/aba74c5e-aab9-4371-84ad-fe67e5321b36)

## Install

Download the package for your system from the release's **Assets** section.
The package must match your exact KWin patch version—KWin effects are not
universal binaries.

Check the installed package before choosing an asset:

```sh
pacman -Q kwin                  # CachyOS or SteamOS
dpkg-query -W kwin-wayland     # Kubuntu
rpm -q kwin                     # Fedora KDE
```

The package-manager dependency is intentionally unversioned so AutoScroll
cannot block a KWin or operating-system update. If an update changes KWin,
install the newly matched AutoScroll build before enabling the effect again.

### CachyOS

For the current CachyOS KWin 6.7.5-1.1 package:

```sh
sudo pacman -U ./kwin-autoscroll-0.1.6-1-cachyos-kwin6.7.5-1.1-x86_64.pkg.tar.zst
```

Remove it with:

```sh
sudo pacman -Rns kwin-autoscroll
```

### SteamOS / Steam Deck

This package is for Valve KWin 6.4.3-1.16:

Warning for SteamOS: this effect has not been fully runtime-tested with
Valve's input libraries or Steam Input. Use it with care.

```sh
sudo steamos-readonly disable
sudo pacman -U ./kwin-autoscroll-0.1.6-1-steamos-kwin6.4.3-1.16-x86_64.pkg.tar.zst
sudo steamos-readonly enable
```

SteamOS systems still on KWin `6.4.3-1.13` should instead use the preserved
`kwin-autoscroll-0.1.6-1-steamos-kwin6.4.3-1.13-x86_64.pkg.tar.zst` asset.

Remove it with:

```sh
sudo steamos-readonly disable
sudo pacman -Rns kwin-autoscroll
sudo steamos-readonly enable
```

SteamOS normally keeps the system image read-only. System updates may remove
the package, and an update that changes KWin will need a newly matched build.

### Kubuntu 26.04

This package is for KWin `4:6.6.6-0ubuntu0.1`:

```sh
sudo apt install ./kwin-autoscroll_0.1.6-1-kubuntu26.04-kwin6.6.6_amd64.deb
```

Kubuntu systems still on KWin `4:6.6.5-0ubuntu0.1` should instead use the
preserved `kwin-autoscroll_0.1.6-1-kubuntu26.04-kwin6.6.5_amd64.deb` asset.

Remove it with:

```sh
sudo apt remove kwin-autoscroll
```

### Fedora KDE 44

This package is for Fedora 44 KWin `6.7.5-1.fc44`:

```sh
sudo dnf install ./kwin-autoscroll-0.1.6-3.fc44.x86_64.rpm
```

Remove it with:

```sh
sudo dnf remove kwin-autoscroll
```

Fedora Kinoite can layer the same RPM, followed by a reboot:

```sh
sudo rpm-ostree install ./kwin-autoscroll-0.1.6-3.fc44.x86_64.rpm
systemctl reboot
```

## Turn it on

After installing:

1. Log out and back in.
2. Open **System Settings → Window Management → Desktop Effects**.
3. Find **Auto Scroll** under Accessibility.
4. Enable it and click **Apply**.

The configure button lets you choose an optional activation modifier and one
of three activation behaviors: click to toggle, hold to scroll, or combined
Windows-style click-or-hold. You can also select applications AutoScroll
should leave untouched and adjust the dead zone, maximum speed, acceleration
curve, horizontal scrolling, visual feedback, glyph size, and visual style.
The size picker scales the anchor and directional pointer together. Breeze
Dark is the default; Breeze, Classic, Feather, Orbit, Circuit, and Pulse
remain available as bundled alternatives.

## How it behaves

- **Click to toggle** remains the default: release the middle button and click
  again to stop. **Hold to scroll** stops whenever the middle button is
  released.
- **Click or hold (Windows-style)** keeps AutoScroll toggled when the middle
  button is released inside the dead zone. Moving outside the dead zone while
  holding changes that activation into a hold gesture, and releasing the
  middle button then stops it. Returning to the dead zone does not change the
  gesture back into a toggle.
- Optionally require Control, Meta, Alt, or Shift while middle-clicking. With
  a modifier selected, plain middle-click remains available for opening
  browser links in a new tab, closing browser tabs, or middle-click paste.
- In toggle mode with a modifier, release both the middle button and modifier
  before moving the pointer. In hold mode, keep the middle button down and
  release the modifier. Combined mode follows whichever gesture you perform.
  All three modes avoid generating modifier-sensitive wheel actions such as
  zoom.
- Another click or Escape stops a toggled session immediately; releasing the
  middle button stops hold and combined-hold gestures. The physical scroll
  wheel needs two events, so one accidental detent does not stop AutoScroll.
- Add applications to **Excluded applications** by selecting a running window
  or choosing an installed application. Excluded applications receive normal
  middle clicks and never receive AutoScroll-generated scrolling.
- Leaving the original window also stops it, so another application does not
  receive the generated scrolling.
- Native Wayland applications work best. XWayland applications are best
  effort, and a full Plasma X11 session is not supported.
- While AutoScroll is available in non-excluded application content, that
  middle click is reserved for the effect rather than middle-click paste or
  open-link.

## Building it yourself

The repository can build separate packages for the four supported targets:

Run the complete matrix from an up-to-date Arch-family host. The Arch targets
use clean `devtools` chroots, while the Kubuntu and Fedora targets use rootless
Podman:

```sh
sudo pacman -Syu --needed devtools namcap podman
scripts/check-build-host.sh
```

The Arch builders use `sudo` to create and enter their isolated roots. The
Kubuntu and Fedora builders do not require root.

```sh
scripts/build-target.sh cachyos
scripts/build-target.sh steamos-6.4.3
scripts/build-target.sh kubuntu-26.04
scripts/build-target.sh fedora-44
```

Or build everything:

```sh
scripts/build-all.sh
```

Build roots, downloaded packages, reports, and finished artifacts are kept
outside the repository under:

```text
~/.cache/kwin-autoscroll-builds/
```

Those downloads are reused on later builds, so unchanged target packages do
not need to be fetched again.

The detailed build setup is in [docs/BUILD_MATRIX.md](docs/BUILD_MATRIX.md).
Testing notes are in [docs/TESTING.md](docs/TESTING.md), and the latest changes
are in [docs/RELEASE_NOTES_0.1.6.md](docs/RELEASE_NOTES_0.1.6.md).

## License

KWin AutoScroll is licensed under GPL-2.0-or-later. The visual assets are
original project artwork under the same license.
