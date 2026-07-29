# Kubuntu 26.04 target

The builder starts from the official Ubuntu 26.04 image pinned by registry
digest. Its deb822 source definition enables only the official `resolute` and
`resolute-updates` `main`/`universe` repositories over HTTPS with the Ubuntu
archive keyring.

The image build rejects any `kwin-dev` or `kwin-wayland` candidate other than
`4:6.6.5-0ubuntu0.1`. Qt and the directly used KDE Frameworks development
packages are also pinned. The Debian rules derive the binary package's exact
`kwin-wayland` dependency from the installed `kwin-dev`.

Downloaded `.deb` files and apt repository indexes are mounted from the
target's external `apt-package-cache/` directory. Normal image-layer cache hits
do not invoke apt at all; if the layer must be rebuilt, apt reuses those files
instead of downloading unchanged packages again.
