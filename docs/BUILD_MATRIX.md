<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Reproducible build matrix

KWin AutoScroll has one source tree, but it does not have one universal
binary. KWin embeds its full upstream patch version in the effect factory IID.
Each row below is therefore an independent package built and verified in its
own target userspace.

The 0.1.8 target definitions were checked on 2026-10-19. Machine-readable
reports and checksum files live below the external build root;
`scripts/build-target.sh` records the exact source commit and verifies every
package inside its target userspace.

| Target | Distribution / architecture | KWin package / expected IID | Qt / KDE Frameworks | glibc / compiler | Isolated builder | Result |
|---|---|---|---|---|---|---|
| `cachyos` | CachyOS rolling 2026-10-19 / x86_64-v3 (`x86_64` package label) | `6.8.0-1.1` / `org.kde.kwin.EffectPluginFactory6.8.0` | 6.12.0 / 6.31.0 | `2.44+r50+g1848099f063e-3` / GCC `16.2.1+r23+gd564253eb6c8-1` | devtools root `cachyos/chroot/root`; signed CachyOS and Arch repositories | Build, 10 tests, IID, unified image-item path, linkage, contents, namcap, disposable install/remove verified |
| `steamos-6.4.3` | SteamOS 3.8 / x86_64 | Valve `6.4.3-1.16` / `org.kde.kwin.EffectPluginFactory6.4.3` | 6.9.1 / 6.16.0 | `2.41+r65+ge7c419a29575-1` / GCC `15.1.1+r7+gf36ec88aa85a-1` | devtools root `steamos-6.4.3/chroot/root`; Valve's versioned 3.8 repositories and `holo-keyring 20250801-1` | Build, 10 tests, IID, renderer-factory image-item path, linkage, contents, namcap, disposable install/remove verified |
| `kubuntu-26.04` | Kubuntu 26.04 Resolute / amd64 | `4:6.6.6-0ubuntu0.1` / `org.kde.kwin.EffectPluginFactory6.6.6` | `6.10.2+dfsg-7` / `6.24.0-0ubuntu1` | `2.43-2ubuntu2.4` / GCC metapackage `4:15.2.0-5ubuntu1` | rootless Podman; digest-pinned Ubuntu image | Build, 10 tests, IID, renderer-factory image-item path, linkage, contents, lintian, disposable install/remove verified |
| `kubuntu-26.10` | Kubuntu 26.10 Stonking / amd64 | `4:6.7.5-0ubuntu1` / `org.kde.kwin.EffectPluginFactory6.7.5` | `6.11.2+dfsg-4ubuntu2` / `6.29.0-0ubuntu1` (ECM) | `2.44-1ubuntu2` / GCC metapackage `4:15.2.0-5ubuntu1` | rootless Podman; digest-pinned Ubuntu image | Build, 10 tests, IID, unified image-item path, linkage, contents, lintian, disposable install/remove verified |
| `fedora-44` | Fedora KDE 44 / x86_64 | `6.7.5-1.fc44` / `org.kde.kwin.EffectPluginFactory6.7.5` | `6.11.2-2.fc44` / `6.30.0-1.fc44` | `2.43-8.fc44` / GCC `16.2.1-2.fc44` | rootless Podman; digest-pinned Fedora image | Build, 10 tests, IID, unified image-item path, linkage, contents, rpmlint, disposable install/remove verified |

## Release artifacts

The release set is collected in `dist/0.1.8/`, including source archive,
PKGBUILD, Fedora source RPM, and `SHA256SUMS`. Earlier releases remain in
their own folders.

The default external root is
`$XDG_CACHE_HOME/kwin-autoscroll-builds`, or
`$HOME/.cache/kwin-autoscroll-builds` when `XDG_CACHE_HOME` is unset.

| Target | Artifact relative to the external root | Verification record |
|---|---|---|
| `cachyos` | `cachyos/artifacts/kwin-autoscroll-0.1.8-1-cachyos-kwin6.8.0-1.1-x86_64.pkg.tar.zst` | Rebuild with `scripts/build-target.sh cachyos` after changing source or infrastructure. |
| `steamos-6.4.3` | `steamos-6.4.3/artifacts/kwin-autoscroll-0.1.8-1-steamos-kwin6.4.3-1.16-x86_64.pkg.tar.zst` | Rebuild with `scripts/build-target.sh steamos-6.4.3` after changing source or infrastructure. |
| `kubuntu-26.04` | `kubuntu-26.04/artifacts/kwin-autoscroll_0.1.8-1-kubuntu26.04-kwin6.6.6_amd64.deb` | Rebuild with `scripts/build-target.sh kubuntu-26.04` after changing source or infrastructure. |
| `kubuntu-26.10` | `kubuntu-26.10/artifacts/kwin-autoscroll_0.1.8-1-kubuntu26.10-kwin6.7.5_amd64.deb` | Rebuild with `scripts/build-target.sh kubuntu-26.10`. |
| `fedora-44` | `fedora-44/artifacts/kwin-autoscroll-0.1.8-1.fc44.x86_64.rpm` and matching `.src.rpm` | Rebuild with `scripts/build-target.sh fedora-44` after changing source or infrastructure. |

Two transitional 0.1.6 artifacts are retained for systems that have not yet
received the latest distribution update:

| Distribution package | Artifact | SHA-256 |
|---|---|---|
| SteamOS KWin `6.4.3-1.13` | `steamos-6.4.3/artifacts/kwin-autoscroll-0.1.6-1-steamos-kwin6.4.3-1.13-x86_64.pkg.tar.zst` | `d44fc7d118e9bcb7b4de4e05f3fd82e78102b8ecfc76f44bdb550d9bec55522a` |
| Kubuntu KWin `4:6.6.5-0ubuntu0.1` | `kubuntu-26.04/artifacts/kwin-autoscroll_0.1.6-1-kubuntu26.04-kwin6.6.5_amd64.deb` | `2e29362662ead773eb85ac2dd9ed8caabae1ec558fa941bd0f24f97655e4bc8b` |

The builders use a fixed source epoch and locked target roots so clean rebuilds
remain reproducible. The per-target `reports/artifact-manifest.txt` and
adjacent `.sha256` file are the authoritative outputs of a later run. The
manifests also record the exact source-archive hash, source Git state, target
versions, and builder identity.

## Reproducibility records

- `build-envs/steamos-6.4.3/packages.lock.tsv` records every cached signed
  package archive, its Valve repository URL, version, and SHA-256. Package
  archives remain outside Git.
- `build-envs/kubuntu-26.04/packages.lock.tsv` and
  `build-envs/kubuntu-26.10/packages.lock.tsv` record the complete installed
  builder package set. A different resolution is rejected instead of silently
  replacing this lock. Its external `apt-package-cache/` retains downloaded
  archives and repository indexes for reuse when an image layer must be rebuilt.
- `build-envs/cachyos/packages.lock.tsv` records the complete clean-root
  package set. Its external cache contains matching archives and signatures.
  Repository database checksums are written to the external report directory.
- `build-envs/fedora-44/packages.lock.tsv` records the package set in the
  digest-pinned RPM builder. Its external DNF cache retains the downloaded
  repository metadata and package archives used by later rebuilds.
- `target.env`, the isolated pacman configurations, and the digest-pinned
  Containerfile are the target definitions. Every builder rejects a KWin
  header or package version that differs from its definition.

Runtime package dependencies are deliberately unversioned, preventing an
installed plugin from blocking distribution upgrades. Exact KWin compatibility
continues to be enforced by the target-root and embedded-IID checks above.

Compilation and package-root checks do not exercise compositor input behavior.
See [RUNTIME_TESTING.md](RUNTIME_TESTING.md) for the remaining runtime plan.
