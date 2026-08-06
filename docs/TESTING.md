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
Escape suppression, toggle and hold initiation, two-event wheel cancellation,
application-identity normalization and exclusion, target changes, locking,
closure, teardown, configuration persistence, installed-application discovery,
window-picker parsing, style discovery, SVG rendering at every preset and
scale, and the custom selector properties.
`pluginmetadata_test` also verifies that both plugins can be discovered and
that the effect uses KWin's exact versioned plugin IID.
Package verification requires an unversioned `kwin`/`kwin-wayland` runtime
dependency while retaining exact target-root version and plugin-IID checks.

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
- With each optional activation modifier selected, plain middle-clicks pass
  through and only the exact configured modifier plus middle-click activates.
- Modified activation does not generate scroll events until both the middle
  button and modifier are released, in either release order.
- With click-and-hold enabled and no modifier, scrolling begins while the
  middle button is down and stops when it is released.
- With click-and-hold and a modifier, scrolling begins only after the modifier
  is released while the middle button remains down. Releasing the middle
  button first cancels without generating scrolling.
- Add a native Wayland, XWayland, and Flatpak application through both the
  running-window picker and installed-application chooser. Each entry survives
  Apply and reopening the KCM, and duplicate additions are ignored.
- In every excluded application, middle-click reaches the client normally and
  AutoScroll does not hide the cursor, draw glyphs, or inject axis events.
- Applying an exclusion for the active target or changing initiation mode
  cancels the current session cleanly.
- Exercise Steam and one Gamescope-hosted application and record whether KWin
  exposes a per-application desktop ID or only the outer window class.
- Desktops, panels, decorations, popups, internal KWin UI, locked sessions,
  pointer-constrained clients, and active fullscreen effects do not activate.
- The activating middle-button press and release never reach the client.
- A cancellation click's complete press/release pair is consumed.
- Escape press, repeats, and release are consumed.
- One physical wheel event leaves the session active; the second cancels it.
  Both events still reach the client.
- Native pointer motion continues while auto-scrolling.
- The cursor and anchor remain correctly sized while crossing mixed-DPI
  outputs, and the native cursor is restored after every cancellation path.
- Every visual style renders its matching anchor and directional pointer at
  16, 40, and 72 pixels over both light and dark application content.
- The size popup previews the selected style at each actual size, and changing
  either appearance setting updates an active session without moving its
  anchor or restarting scrolling.
- Turning visual feedback off disables the appearance controls without
  discarding their values; Defaults restores Breeze Dark at 40 pixels.
- On KWin 6.4-6.6 with the OpenGL compositor, both the anchor and directional
  glyph are visible; this specifically exercises the renderer-created image
  item compatibility path.
- Settings, Defaults, and Reset work, and saved settings take effect without a
  compositor restart.
- Logout/login loads the installed plugin without KWin warnings or crashes.

Never perform the first manual run in a session containing unsaved work. A
binary KWin effect executes inside the compositor process.
