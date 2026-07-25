# Contributing

Contributions are welcome.

## Development workflow

1. Install the dependencies listed in the README.
2. Configure an out-of-source debug build with tests enabled.
3. Keep compositor-independent behavior in `scrollengine` or `sessionstate` so
   it remains unit-testable.
4. Run:

   ```sh
   cmake --build build
   ctest --test-dir build --output-on-failure
   clang-format --dry-run --Werror src/*.{h,cpp} tests/*.cpp kcm/*.{h,cpp}
   ```

5. For packaging changes, run `namcap` on Arch artifacts and `lintian` on
   Debian artifacts.

KWin does not provide binary compatibility for effects. Any use of newly added
KWin APIs must update the minimum supported version and the distribution
package constraints.

## Safety expectations

Input filtering runs inside the compositor. Changes must preserve these
invariants:

- synthetic events are recognized and never recursively reprocessed;
- every consumed button press has a consumed release;
- cancellation always sends required axis-stop events;
- the native cursor is restored on every cancellation and teardown path; and
- no auto-scroll input is generated on the lock screen or outside the
  originating application window.

Add SPDX copyright and license headers to new source files. Do not commit build
trees, generated Qt/KConfig files, package work directories, logs, or crash
dumps.
