<!--
SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Reproducible build matrix

KWin AutoScroll has one source tree, but it does not have one universal
binary. KWin embeds its full upstream patch version in the effect factory IID.
Each row below is therefore an independent package built and verified in its
own target userspace.

This snapshot was produced on 2026-08-04 from Git commit
`62d7d96` plus the reviewed working-tree
changes. Machine-readable reports and checksum files live below the external
build root; `scripts/build-target.sh` refreshes them after a successful build.

## CachyOS host inventory

The x86-64 workstation had `kwin 6.7.3-1.1` (upstream 6.7.3),
`qt6-base 6.11.1-1.1`, KDE Frameworks/ECM `6.28.0-1`, glibc
`2.43+r37+gfdf10644d6ee-2`, GCC
`16.1.1+r581+gb73ad535acaa-1`, CMake `4.4.1-1.1`, and pacman
`7.1.0.r9.g54d9411-4`. The host-side builders were devtools `1:1.5.1-1`,
Podman `6.0.2-2.1`, namcap `3.6.0-3`, arch-install-scripts `31-1`,
libarchive `3.8.9-1.1`, GnuPG `2.4.9-2.1`, binutils
`2.46.1+r3+g046eeeef4721-2`, GNU Make `4.4.1-3.1`, Ninja `1.13.2-3.1`,
and pkgconf `3.0.4-1.1`.

These are orchestration versions, not the compiler or libc used in release
artifacts. Each row below was compiled wholly inside its isolated builder.

| Target | Distribution / architecture | KWin package / expected IID | Qt / KDE Frameworks | glibc / compiler | Isolated builder | Result |
|---|---|---|---|---|---|---|
| `cachyos` | CachyOS rolling 2026-07-30 / x86_64-v3 (`x86_64` package label) | `6.7.3-1.1` / `org.kde.kwin.EffectPluginFactory6.7.3` | 6.11.1 / 6.28.0 | `2.44+r3+g0b05bc142249-1` / GCC `16.1.1+r581+gb73ad535acaa-1` | devtools root `cachyos/chroot/root`; signed CachyOS and Arch repositories | Build, 9 tests, IID, unified image-item path, linkage, contents, namcap, disposable install/remove verified |
| `steamos-6.4.3` | SteamOS 3.8 / x86_64 | Valve `6.4.3-1.13` / `org.kde.kwin.EffectPluginFactory6.4.3` | 6.9.1 / 6.16.0 | `2.41+r65+ge7c419a29575-1` / GCC `15.1.1+r7+gf36ec88aa85a-1` | devtools root `steamos-6.4.3/chroot/root`; Valve's versioned 3.8 repositories and `holo-keyring 20250801-1` | Build, 9 tests, IID, renderer-factory image-item path, linkage, contents, namcap, disposable install/remove verified |
| `kubuntu-26.04` | Kubuntu 26.04 Resolute / amd64 | `4:6.6.5-0ubuntu0.1` / `org.kde.kwin.EffectPluginFactory6.6.5` | `6.10.2+dfsg-7` / `6.24.0-0ubuntu1` | `2.43-2ubuntu2.3` / GCC metapackage `4:15.2.0-5ubuntu1` | rootless Podman; Ubuntu image digest `sha256:7c2884fd32770fc6c173b78e0dc2278a2851d89f5447919edbc45475ac55dd6a`; builder image `de1bc8831c3a701755ef5f891235034dc6aed434eb6805de9af391d811d4de4c` | Build, 9 tests, IID, renderer-factory image-item path, linkage, contents, lintian, disposable install/remove verified |

## Last validated artifacts

The default external root is
`$XDG_CACHE_HOME/kwin-autoscroll-builds`, or
`$HOME/.cache/kwin-autoscroll-builds` when `XDG_CACHE_HOME` is unset.

| Target | Artifact relative to the external root | Verification record |
|---|---|---|
| `cachyos` | `cachyos/artifacts/kwin-autoscroll-0.1.5-1-cachyos-kwin6.7.3-1.1-x86_64.pkg.tar.zst` | Rebuild with `scripts/build-target.sh cachyos` after changing source or infrastructure. |
| `steamos-6.4.3` | `steamos-6.4.3/artifacts/kwin-autoscroll-0.1.5-1-steamos-kwin6.4.3-1.13-x86_64.pkg.tar.zst` | Rebuild with `scripts/build-target.sh steamos-6.4.3` after changing source or infrastructure. |
| `kubuntu-26.04` | `kubuntu-26.04/artifacts/kwin-autoscroll_0.1.5-1-kubuntu26.04-kwin6.6.5_amd64.deb` | Rebuild with `scripts/build-target.sh kubuntu-26.04` after changing source or infrastructure. |

The builders use a fixed source epoch and locked target roots so clean rebuilds
remain reproducible. The per-target `reports/artifact-manifest.txt` and
adjacent `.sha256` file are the authoritative outputs of a later run. The
manifests also record the exact source-archive hash, source Git state, target
versions, and builder identity.

## Reproducibility records

- `build-envs/steamos-6.4.3/packages.lock.tsv` records every cached signed
  package archive, its Valve repository URL, version, and SHA-256. Package
  archives remain outside Git.
- `build-envs/kubuntu-26.04/packages.lock.tsv` records the complete installed
  builder package set. A different resolution is rejected instead of silently
  replacing this lock. Its external `apt-package-cache/` retains 715 `.deb`
  archives and repository indexes for reuse when an image layer must be
  rebuilt.
- `build-envs/cachyos/packages.lock.tsv` records the complete clean-root
  package set. Its external cache contains all 394 matching archives and
  signatures. Repository database checksums are written to the external report
  directory.
- `target.env`, the isolated pacman configurations, and the digest-pinned
  Containerfile are the target definitions. Every builder rejects a KWin
  header or package version that differs from its definition.

Compilation and package-root checks do not exercise compositor input behavior.
See [RUNTIME_TESTING.md](RUNTIME_TESTING.md) for the remaining runtime plan.
