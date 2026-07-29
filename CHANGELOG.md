# Changelog

All notable changes to this project will be documented here.

## 0.1.2 — 2026-07-29

- Expanded the supported build range to KWin 6.4.3 and newer while retaining
  exact-patch binary packages for every KWin target.
- Made output-scale lookup compatible with the `Output *` return type used by
  KWin 6.4/6.5 and the `LogicalOutput *` return type used by KWin 6.6 and
  newer.
- Fixed missing anchor and direction glyphs on KWin 6.4-6.6 by asking the
  active scene renderer to create its backend-specific image items. KWin 6.7
  and newer continue to use the unified image item directly.
- Added isolated CachyOS, SteamOS 6.4.3, and Kubuntu 26.04 package builds with
  version, plugin-IID, linkage, contents, package-manager, and linter checks.
- Added locked target package manifests and reproducible, byte-stable release
  artifacts without changing the existing compositor input-filter behavior.

## 0.1.1 — 2026-07-27

- Fixed glyph rendering on scaled displays.

## 0.1.0 — 2026-07-25

- Initial Plasma 6.7 Wayland implementation.
- Middle-click activation with distance-based vertical and horizontal scrolling.
- Windows-like cancellation and safe input-pair suppression.
- DPI-aware anchor and eight-direction cursor feedback.
- Desktop Effects configuration module.
- Reliable live configuration reloads, including acceleration exponents below
  1.0.
- Gentle defaults: a 24-unit dead zone, 800 units/second maximum speed, and a
  1.6 acceleration exponent.
- Automated engine, state-machine, activation-policy, and plugin-metadata tests.
- Arch/CachyOS and Debian-family Plasma packaging.
