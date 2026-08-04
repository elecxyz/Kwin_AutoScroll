<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# KWin AutoScroll 0.1.5

KWin AutoScroll 0.1.5 can leave selected applications completely untouched
and adds an optional hold-to-scroll initiation mode.

## Application exclusions

The configuration page can add an application by selecting one of its running
windows or choosing it from the installed application list. AutoScroll does
not consume middle clicks, display glyphs, or inject scrolling in excluded
applications. Desktop-file identifiers are used where available, with the
window resource class retained as an XWayland fallback.

## Initiation behavior

Click-to-toggle remains the default. Enabling **Click and hold to auto-scroll**
keeps AutoScroll active only while the middle button is held. When an
activation modifier is configured, scrolling waits for that modifier to be
released while the middle button remains down, avoiding modified-wheel actions
such as zoom.

## Compatibility

The release remains source-compatible with KWin 6.4.3 and newer and includes
separate packages for the pinned CachyOS, SteamOS 6.4.3, and Kubuntu 26.04
targets.
