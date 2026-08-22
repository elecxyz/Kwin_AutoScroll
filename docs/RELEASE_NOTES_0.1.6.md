<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# KWin AutoScroll 0.1.6

KWin AutoScroll 0.1.6 adds a combined Windows-style activation behavior while
preserving the original click-to-toggle and hold-to-scroll choices.

## Three activation behaviors

- **Click to toggle** keeps scrolling active after the middle button is
  released and stops on another click, Escape, or the existing cancellation
  inputs.
- **Hold to scroll** remains active only while the middle button is held.
- **Click or hold (Windows-style)** decides from the gesture. Releasing inside
  the configured dead zone toggles AutoScroll; moving outside the dead zone
  while holding makes the gesture hold-to-scroll, so release stops it.

Once a combined gesture leaves the dead zone it remains a hold gesture, even
if the pointer returns to the anchor before release. Optional activation
modifiers continue to prevent modified scrolling actions in all three modes.

## Configuration compatibility

Existing installations retain their previous behavior. The old
`HoldToScroll=false` preference maps to **Click to toggle**, and
`HoldToScroll=true` maps to **Hold to scroll**. Combined mode is an explicit
new choice.

## Packages

The release includes separately built and verified packages for CachyOS KWin
6.7.4-5.1, SteamOS KWin 6.4.3-1.15, and Kubuntu 26.04 KWin
4:6.6.6-0ubuntu0.1. Runtime KWin dependencies remain unversioned so the plugin
cannot block operating-system upgrades.

Verified transitional assets for SteamOS KWin 6.4.3-1.13 and Kubuntu KWin
4:6.6.5-0ubuntu0.1 are also retained for systems that have not received the
latest distribution update. Users should select the asset matching their
installed KWin package.
