# Fedora 45 build target

This target builds KWin AutoScroll for Fedora KDE 45 x86_64 in a rootless,
digest-pinned Podman image. It is pinned to Fedora's KWin `6.8.2-3.fc45`
package and the embedded effect IID
`org.kde.kwin.EffectPluginFactory6.8.2`.

The builder rejects unexpected KWin, Qt, KDE Frameworks, glibc, compiler, or
installed-package versions. The RPM keeps its runtime `kwin` dependency
unversioned so it cannot block a Fedora system update.

Build it from the repository root:

```sh
scripts/build-target.sh fedora-45
```

The binary RPM, source RPM, checksums, logs, package cache, and manifest are
written below `$XDG_CACHE_HOME/kwin-autoscroll-builds/fedora-45`, or below
`$HOME/.cache/kwin-autoscroll-builds/fedora-45` when `XDG_CACHE_HOME` is not
set.

The 5 November 2026 build keeps AutoScroll at version 0.1.8 and uses RPM
release `2.fc45`. Its release assets are:

- `kwin-autoscroll-0.1.8-2.fc45.x86_64.rpm`
- `kwin-autoscroll-0.1.8-2.fc45.src.rpm`

These assets are collected in `dist/0.1.8-fedora-kwin6.8.2/`, together with
the other Fedora build and `SHA256SUMS`. See the
[installation instructions](../../README.md#fedora-kde-45) and
[0.1.8 release notes](../../docs/RELEASE_NOTES_0.1.8.md).
