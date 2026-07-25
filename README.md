# KWin AutoScroll

KWin AutoScroll is a binary effect for KDE Plasma 6 Wayland. It adds
Windows-style continuous scrolling: middle-click in an application, move the
pointer away from the anchor, and the distance and direction control scrolling
speed.

Version 0.1 targets KWin 6.7 or newer. Native Wayland applications are
supported; XWayland applications are best effort. A full Plasma X11 session is
not supported.

## Quick install from a release

Download the package for your distribution from the release's **Assets**
section. Do not extract it.

On Arch Linux or CachyOS, install the downloaded package with:

```sh
sudo pacman -U ./kwin-autoscroll-0.1.0-1-x86_64.pkg.tar.zst
```

On Kubuntu or another Ubuntu-based system running Plasma Wayland, install the
downloaded package with:

```sh
sudo apt install ./kwin-autoscroll_0.1.0-1_amd64.deb
```

The package must have been built for your exact KWin patch version. Check yours
with `kwin_wayland --version`; the package manager will reject an incompatible
package. The system also needs a repository that provides KWin 6.7 or newer.
This plugin does not work in Ubuntu's default GNOME session.

After installation, log out and back in. Open **System Settings → Window
Management → Desktop Effects**, find **Auto Scroll** under Accessibility,
enable it, and apply.

## Quick removal

Disable **Auto Scroll** in Desktop Effects first. On Arch Linux or CachyOS:

```sh
sudo pacman -Rns kwin-autoscroll
```

On Kubuntu or another Ubuntu-based Plasma system:

```sh
sudo apt remove kwin-autoscroll
```

Log out and back in after removal so KWin unloads the binary plugin completely.

## Important behavior

The effect cannot ask an application whether the control beneath the pointer is
scrollable. When enabled, an unmodified middle click anywhere in normal
application content is therefore reserved for AutoScroll, including controls
that do not scroll. Middle-click paste and open-link actions remain available
on the desktop, panels, decorations, popups, and other excluded surfaces, but
not in eligible application content.

Auto-scroll ends when you:

- click any mouse button;
- press Escape;
- use a physical scroll wheel;
- leave the application window where scrolling started; or
- close the window, lock the screen, disable the effect, or activate another
  fullscreen KWin effect.

The activating and cancelling click pairs are consumed so applications never
receive unmatched button releases. A physical wheel event cancels auto-scroll
and is then passed to the application.

## Build requirements

KWin binary effects use a
[private, versioned compositor interface](https://invent.kde.org/plasma/kwin/-/blob/v6.7.3/src/plugin.h).
The plugin must be rebuilt against the exact KWin patch release that will load
it.

On Arch Linux or CachyOS:

```sh
sudo pacman -S --needed base-devel git cmake extra-cmake-modules ninja \
    namcap vulkan-headers kwin
```

On Debian, Kubuntu, or another Debian/Ubuntu-based Plasma system with Plasma
6.7+ development packages:

```sh
sudo apt install build-essential cmake extra-cmake-modules ninja-build \
    kwin-dev libkf6config-dev libkf6configwidgets-dev \
    libkf6coreaddons-dev libkf6i18n-dev libkf6kcmutils-dev \
    libkf6widgetsaddons-dev qt6-base-dev qt6-svg-dev libvulkan-dev
```

Older distributions whose `kwin-dev` is below 6.7 cannot build this release.

## Build and test

Always use an out-of-source build:

```sh
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DBUILD_TESTING=ON \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
ctest --test-dir build --output-on-failure
```

The complete automated and disposable-session validation matrix is documented
in [docs/TESTING.md](docs/TESTING.md). Contribution conventions are in
[CONTRIBUTING.md](CONTRIBUTING.md).

To inspect the staged install without changing the system:

```sh
DESTDIR="$PWD/build/stage" cmake --install build
find build/stage -type f
```

## Install and enable

Packaging is preferred because it tracks the exact KWin dependency. For a
direct source install:

```sh
sudo cmake --install build
```

Then open **System Settings → Window Management → Desktop Effects**, find
**Auto Scroll** in Accessibility, enable it, and apply. Log out and back in if
the newly installed binary effect is not listed immediately.

The effect is disabled by default. Its configure button exposes:

- dead-zone radius;
- maximum scroll speed;
- acceleration curve;
- horizontal scrolling; and
- anchor/directional visual feedback.

The defaults are a 24-unit dead zone, 800 units/second maximum speed, and a 1.6
curve. Values below 1.0 are supported; they make scrolling react more strongly
just outside the dead zone, while values above 1.0 make that region gentler.

## Uninstall

Disable the effect first. If it was installed from an OS package, remove that
package. A direct CMake install can be removed using the generated manifest:

```sh
sudo cmake --build build --target uninstall
```

Log out and back in after removing a loaded binary effect.

## Packaging and releases

- `packaging/arch/PKGBUILD` builds a local release tarball and pins KWin 6.7.3.
- `debian/` contains Debian-family source packaging and derives an exact
  `kwin-wayland` runtime dependency from the build environment.
- `scripts/make-release.sh` creates a reproducible source archive and a release
  PKGBUILD with its real SHA-256 checksum in `dist/`.

Create release artifacts with:

```sh
scripts/make-release.sh
cd dist
makepkg --cleanbuild
namcap PKGBUILD kwin-autoscroll-*.pkg.tar.*
```

## Troubleshooting

- **The effect is missing after installation:** verify that the plugin is under
  the Qt 6 `kwin/effects/plugins` directory and that its embedded KWin version
  matches `kwin_wayland --version`.
- **KWin reports a mismatched plugin version:** rebuild and reinstall after
  every KWin update, including patch releases.
- **Scrolling stops at the window edge:** this is intentional; Wayland routes
  axis events to the surface under the pointer, so stopping prevents another
  application from receiving synthetic input.
- **An application does not scroll smoothly:** it may not support Wayland
  continuous-axis events. XWayland behavior is application-dependent.

## License

KWin AutoScroll is licensed under GPL-2.0-or-later. The visual assets are
original project artwork under the same license.
