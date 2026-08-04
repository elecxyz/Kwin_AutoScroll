# SteamOS KWin 6.4.3 target

This target uses Valve's official SteamOS 3.8 `jupiter-3.8`, `holo-3.8`,
`core-3.8`, and `extra-3.8` repositories. The repository order selects
Valve's `kwin 6.4.3-1.13`; the same repository set supplies Qt 6.9.1, KDE
Frameworks 6.16, glibc 2.41, GCC 15.1, binutils, and the rest of the complete
build userspace.

The setup downloads `holo-keyring` over TLS from Valve's repository, verifies
its pinned SHA-256, confirms the expected Valve CI signing fingerprint, and
uses `pacman-key --populate-from` in a target-only keyring. Repository package
signatures remain required.

Installing an additional system package on SteamOS requires temporarily
making the normally read-only OS writable. SteamOS updates may remove the
package, and every KWin patch update requires a matching rebuild. Keep the
generated package and report somewhere outside the mutable system partition.
