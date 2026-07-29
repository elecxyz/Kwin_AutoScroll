<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Testing KWin AutoScroll

Run automated tests from an out-of-tree build:

```sh
cmake -S . -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The unit tests cover the speed curve, dead zone, signs, diagonal motion,
maximum-speed cap, horizontal disablement, elapsed-time integration,
fractional deltas, axis-stop generation, activation policy, click suppression,
Escape suppression, wheel cancellation, target changes, locking, closure, and
teardown. `pluginmetadata_test` also verifies that both plugins can be
discovered and that the effect uses KWin's exact versioned plugin IID.

## Manual Wayland matrix

Install into a nested or disposable Plasma 6.4.3-or-newer Wayland session
whose exact KWin patch release matches the package. Enable KWin AutoScroll
under **System Settings → Window Management → Desktop Effects →
Accessibility**.

For every application class below, verify activation, vertical scrolling,
horizontal scrolling where supported, all eight visual directions, returning
to the dead zone, and cancellation by click, Escape, physical wheel, leaving
the originating window, closing the target, screen locking, and disabling the
effect:

- Qt 6 application
- GTK application
- Chromium- or Firefox-based browser
- Electron application
- XWayland application (best effort)

Also verify:

- A plain middle-click activates, while modified middle-clicks pass through.
- Desktops, panels, decorations, popups, internal KWin UI, locked sessions,
  pointer-constrained clients, and active fullscreen effects do not activate.
- The activating middle-button press and release never reach the client.
- A cancellation click's complete press/release pair is consumed.
- Escape press, repeats, and release are consumed.
- The physical wheel event that cancels a session still reaches the client.
- Native pointer motion continues while auto-scrolling.
- The cursor and anchor remain correctly sized while crossing mixed-DPI
  outputs, and the native cursor is restored after every cancellation path.
- On KWin 6.4-6.6 with the OpenGL compositor, both the anchor and directional
  glyph are visible; this specifically exercises the renderer-created image
  item compatibility path.
- Settings, Defaults, and Reset work, and saved settings take effect without a
  compositor restart.
- Logout/login loads the installed plugin without KWin warnings or crashes.

Never perform the first manual run in a session containing unsaved work. A
binary KWin effect executes inside the compositor process.
