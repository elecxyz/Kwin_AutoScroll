# CachyOS target

This target is locked to the KWin package installed when the target was
created: `kwin 6.7.3-1.1` from `cachyos-extra-v3`. The clean chroot uses the
official CachyOS x86-64-v3 repositories and official Arch repositories without
reading the host's pacman configuration.

The package is compiled with x86-64-v3 flags but retains Arch's conventional
`x86_64` package architecture label. Repository packages remain
signature-required.
