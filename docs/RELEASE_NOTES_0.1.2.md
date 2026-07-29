<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# KWin AutoScroll 0.1.2

Released: 2026-07-29

KWin AutoScroll 0.1.2 expands the supported source range and introduces
reproducible, exact-version packages for three KWin environments. The
auto-scroll interaction and existing compositor input-filter implementation
remain unchanged.

## Highlights

- Supports source builds with KWin 6.4.3 and newer.
- Adds a SteamOS / Steam Deck package built against Valve KWin 6.4.3-1.13,
  Qt 6.9.1, and KDE Frameworks 6.16.
- Adds a Kubuntu 26.04 package built against KWin
  `4:6.6.5-0ubuntu0.1`.
- Adds a clean-chroot CachyOS package for KWin 6.7.3-1.1.
- Produces a separate package for every KWin patch version because KWin effect
  binaries embed that exact version in their plugin IID.

## Compatibility changes

- Lowered the CMake KWin minimum from 6.7.0 to 6.4.3.
- Made output-scale lookup compile with both the `KWin::Output *` return type
  used by KWin 6.4/6.5 and the `KWin::LogicalOutput *` type used by newer
  releases.
- Fixed invisible anchor and direction glyphs on KWin 6.4-6.6. Those releases
  require image items to be created by the active scene renderer so the
  OpenGL backend receives an `ImageItemOpenGL`; KWin 6.7 and newer use the
  unified `ImageItem` implementation.
- Lowered the Debian `kwin-dev` build constraint while retaining the exact
  generated `kwin-wayland` runtime dependency.
- Kept the existing `InputEventFilter` implementation; this release does not
  switch exclusively to the newer KWin 6.7 pointer-effect callbacks.

## Build and verification

- Added isolated CachyOS and SteamOS devtools chroots.
- Added a digest-pinned, rootless Ubuntu 26.04 Podman builder.
- Added complete target package locks, exact KWin version gates, reproducible
  source epochs, and target-specific artifact names.
- Every package is tested for its embedded IID, KWin dependency, installation
  paths, target-root shared-library resolution, and disposable
  installation/removal.
- The complete five-test automated suite runs for every target.
- Arch-family packages run namcap; the Debian package runs lintian.

## Installation warning

Install only the package matching the system's exact KWin patch release.
SteamOS normally uses a read-only system partition; installing the SteamOS
package requires deliberately making it writable, and a SteamOS update may
remove the package or require another exact-version rebuild.

## Runtime validation

Compilation, packaging, metadata, linkage, and disposable installation are
validated. Live compositor input testing remains a separate step using a
nested CachyOS session, a Kubuntu snapshot VM, and a real Steam Deck or
sufficiently faithful disposable SteamOS environment.

## Production artifacts

| Target | Package | SHA-256 |
|---|---|---|
| CachyOS | `kwin-autoscroll-0.1.2-2-cachyos-kwin6.7.3-1.1-x86_64.pkg.tar.zst` | `2b019d65331ef40ac20d6a297a18ae744195542804f511ce4bfa09153a240f9b` |
| SteamOS | `kwin-autoscroll-0.1.2-2-steamos-kwin6.4.3-1.13-x86_64.pkg.tar.zst` | `4f4ba152e1049ce470883e614bb7f707b00d243fb6e02f9922ab7582babb63a6` |
| Kubuntu 26.04 | `kwin-autoscroll_0.1.2-2-kubuntu26.04-kwin6.6.5_amd64.deb` | `671a17e9f81e385f856c822aa8fbfe406cb1b0d98921e7d57c04da2eafe94af2` |

All three packages passed the complete automated test suite, exact KWin IID
and dependency checks, version-appropriate image-item construction checks,
target-root linkage inspection, package-content inspection, linting, and
disposable installation/removal.
