# Kubuntu 26.10 target

The builder starts from the official Ubuntu 26.10 image pinned by registry
digest. Its deb822 source definition enables only the official `stonking` and
`stonking-updates` `main`/`universe` repositories over HTTPS with the Ubuntu
archive keyring.

The image build rejects any `kwin-dev` or `kwin-wayland` candidate other than
`4:6.7.5-0ubuntu1`. Qt and the directly used KDE Frameworks development
packages are also pinned. The binary package intentionally uses an unversioned
`kwin-wayland` dependency so an installed plugin cannot block system updates;
the builder still enforces the exact KWin target and plugin IID.

Downloaded `.deb` files and apt repository indexes are mounted from the
target's external `apt-package-cache/` directory. Normal image-layer cache hits
do not invoke apt at all; if the layer must be rebuilt, apt reuses those files
instead of downloading unchanged packages again.
