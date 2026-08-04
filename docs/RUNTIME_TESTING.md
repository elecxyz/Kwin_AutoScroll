<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Runtime validation plan

The isolated builds prove compilation, exact plugin metadata, linkage, package
contents, dependencies, and package-manager installation/removal. They do not
prove that a live compositor handles pointer input correctly.

## CachyOS

1. Create a disposable user account or snapshot and start a nested KWin
   Wayland session with the same `kwin 6.7.3-1.1` package.
2. Install the CachyOS artifact only inside that disposable environment.
3. Run the application and cancellation matrix in [TESTING.md](TESTING.md),
   including mixed-DPI cursor scaling and XWayland best-effort behavior.
4. Save the nested compositor log, then remove the package and discard the
   environment.

## Kubuntu 26.04

1. Create a QEMU/KVM Kubuntu 26.04 VM using a temporary overlay or snapshot.
2. Confirm `kwin-wayland` is exactly `4:6.6.5-0ubuntu0.1`.
3. Install the matching `.deb`, log into Plasma Wayland, and execute the full
   manual matrix.
4. Capture the journal and KWin output, remove the package, and revert the
   snapshot.

Obtaining the installation image is intentionally a separate, user-approved
step because it is large and outside compilation validation.

## SteamOS / Steam Deck

1. Copy the SteamOS artifact to a Deck whose installed KWin is exactly Valve
   `6.4.3-1.13` from the SteamOS 3.8 repositories.
2. Take the normal recovery/snapshot precautions, temporarily make the
   normally read-only OS writable, and install with `pacman -U`.
3. Run the manual matrix in Gaming Mode and Desktop Mode where applicable,
   paying particular attention to touchpad/mouse event routing.
4. Remove the package or restore the immutable system state after testing.
