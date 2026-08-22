#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

(($# == 7)) || {
    printf 'usage: %s TOPDIR VERSION RPM_RELEASE KWIN_PACKAGE QT_PACKAGE KF_PACKAGE KWIN_UPSTREAM\n' "${0##*/}" >&2
    exit 2
}

topdir=$1
version=$2
rpm_release=$3
kwin_package=$4
qt_package=$5
kf_package=$6
kwin_upstream=$7

installed_version() {
    rpm -q --qf '%{VERSION}-%{RELEASE}' "$1"
}

test "$(installed_version kwin)" = "${kwin_package}"
test "$(installed_version kwin-devel)" = "${kwin_package}"
test "$(installed_version qt6-qtbase-devel)" = "${qt_package}"
test "$(installed_version extra-cmake-modules)" = "${kf_package}"
test "$(sed -n 's/^#define KWIN_PLUGIN_VERSION_STRING "\(.*\)"/\1/p' /usr/include/kwin/config-kwin.h)" = "${kwin_upstream}"

rpmbuild -ba \
    --define "_topdir ${topdir}" \
    --define "_buildhost kwin-autoscroll-fedora-builder" \
    --define "use_source_date_epoch_as_buildtime 1" \
    "${topdir}/SPECS/kwin-autoscroll.spec"

package=$(find "${topdir}/RPMS" -type f \
    -name "kwin-autoscroll-${version}-${rpm_release}.x86_64.rpm" -print -quit)
source_package=$(find "${topdir}/SRPMS" -type f \
    -name "kwin-autoscroll-${version}-${rpm_release}.src.rpm" -print -quit)
test -n "${package}"
test -n "${source_package}"

rpmlint --rpmlintrc "${topdir}/SOURCES/kwin-autoscroll.rpmlintrc" \
    "${source_package}" "${package}"

printf '\nRPM requirements:\n'
rpm -qp --requires "${package}"
printf '\nRPM contents:\n'
rpm -qlp "${package}"

dnf -y --disablerepo='*' install "${package}"
effect=/usr/lib64/qt6/plugins/kwin/effects/plugins/autoscroll.so
kcm=/usr/lib64/qt6/plugins/kwin/effects/configs/kwin_autoscroll_config.so
test -f "${effect}"
test -f "${kcm}"

printf '\nELF dependencies resolved inside target image:\n'
effect_ldd=$(ldd "${effect}")
kcm_ldd=$(ldd "${kcm}")
printf '%s\n%s\n' "${effect_ldd}" "${kcm_ldd}"
case "${effect_ldd}${kcm_ldd}" in
    *"not found"*)
        printf 'unresolved target-image shared library\n' >&2
        exit 1
        ;;
esac

rpm -e kwin-autoscroll
if rpm -q kwin-autoscroll >/dev/null 2>&1; then
    printf 'RPM removal verification failed\n' >&2
    exit 1
fi

rpm -qa --qf '%{NAME}\t%{EPOCH}:%{VERSION}-%{RELEASE}.%{ARCH}\n' |
    sort > /work/installed-packages.tsv
