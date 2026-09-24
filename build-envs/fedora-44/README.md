# Fedora 44 build target

This target builds KWin AutoScroll for Fedora KDE 44 x86_64 in a rootless,
digest-pinned Podman image. It is pinned to Fedora's KWin `6.7.5-1.fc44`
package and the embedded effect IID
`org.kde.kwin.EffectPluginFactory6.7.5`.

The builder rejects unexpected KWin, Qt, KDE Frameworks, glibc, compiler, or
installed-package versions. The RPM keeps its runtime `kwin` dependency
unversioned so it cannot block a Fedora system update.

Build it from the repository root:

```sh
scripts/build-target.sh fedora-44
```

The binary RPM, source RPM, checksums, logs, package cache, and manifest are
written below `$XDG_CACHE_HOME/kwin-autoscroll-builds/fedora-44`, or below
`$HOME/.cache/kwin-autoscroll-builds/fedora-44` when `XDG_CACHE_HOME` is not
set.
