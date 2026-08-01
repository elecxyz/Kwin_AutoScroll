# KWin AutoScroll

I built KWin AutoScroll out of frustration that this did not already exist on
Plasma Wayland.

Middle-click in an application, move the pointer away from the anchor, and the
distance and direction control the scrolling speed. Move back into the dead
zone to pause, or click again to stop.




## Install

Download the package for your system from the release's **Assets** section.
The package must match your exact KWin patch version—KWin effects are not
universal binaries.

### CachyOS

For the current CachyOS KWin 6.7.3-1.1 package:

```sh
sudo pacman -U ./kwin-autoscroll-0.1.4-1-cachyos-kwin6.7.3-1.1-x86_64.pkg.tar.zst
```

Remove it with:

```sh
sudo pacman -Rns kwin-autoscroll
```

### SteamOS / Steam Deck

This package is for Valve KWin 6.4.3-1.13:

Warning for SteamOS - I've not fully verified how this addon will play with Valve's input libraries or wether it plays nice with steam input. Use at your own risk I guess.

```sh
sudo steamos-readonly disable
sudo pacman -U ./kwin-autoscroll-0.1.4-1-steamos-kwin6.4.3-1.13-x86_64.pkg.tar.zst
sudo steamos-readonly enable
```

Remove it with:

```sh
sudo steamos-readonly disable
sudo pacman -Rns kwin-autoscroll
sudo steamos-readonly enable
```

SteamOS normally keeps the system image read-only. System updates may remove
the package, and an update that changes KWin will need a newly matched build.

### Kubuntu 26.04

This package is for KWin `4:6.6.5-0ubuntu0.1`:

```sh
sudo apt install ./kwin-autoscroll_0.1.4-1-kubuntu26.04-kwin6.6.5_amd64.deb
```

Remove it with:

```sh
sudo apt remove kwin-autoscroll
```

## Turn it on

After installing:

1. Log out and back in.
2. Open **System Settings → Window Management → Desktop Effects**.
3. Find **Auto Scroll** under Accessibility.
4. Enable it and click **Apply**.

The configure button lets you choose an optional activation modifier and
adjust the dead zone, maximum speed, acceleration curve, horizontal scrolling,
visual feedback, glyph size, and visual style. The size picker scales the
anchor and directional pointer together. Breeze Dark is the default; Breeze,
Classic, Feather, Orbit, Circuit, and Pulse remain available as bundled
alternatives.

## How it behaves

- By default, plain middle-click starts auto-scroll in normal application
  content.
- Optionally require Control, Meta, Alt, or Shift while middle-clicking. With
  a modifier selected, plain middle-click remains available for opening
  browser links in a new tab, closing browser tabs, or middle-click paste.
- When using a modifier, release both the middle button and modifier before
  moving the pointer. AutoScroll waits for the complete chord to be released
  so generated wheel events do not become zoom or horizontal-scroll actions.
- Another click or Escape stops it immediately. The physical scroll wheel
  needs two events, so one accidental detent does not stop AutoScroll.
- Leaving the original window also stops it, so another application does not
  receive the generated scrolling.
- Native Wayland applications work best. XWayland applications are best
  effort, and a full Plasma X11 session is not supported.
- While AutoScroll is available in an application's content, that middle click
  is reserved for the effect rather than middle-click paste or open-link.

## Building it yourself

The repository can build separate packages for the three supported targets:

Run the complete matrix from an up-to-date Arch-family host. The Arch targets
use clean `devtools` chroots, and the Kubuntu target uses rootless Podman:

```sh
sudo pacman -Syu --needed devtools namcap podman
scripts/check-build-host.sh
```

The Arch builders use `sudo` to create and enter their isolated roots. The
Kubuntu builder does not require root.

```sh
scripts/build-target.sh cachyos
scripts/build-target.sh steamos-6.4.3
scripts/build-target.sh kubuntu-26.04
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
are in [docs/RELEASE_NOTES_0.1.4.md](docs/RELEASE_NOTES_0.1.4.md).

## License

KWin AutoScroll is licensed under GPL-2.0-or-later. The visual assets are
original project artwork under the same license.
