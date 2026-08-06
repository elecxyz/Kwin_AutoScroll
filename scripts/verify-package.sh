#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# shellcheck source=scripts/lib/build-common.sh
source "${project_root}/scripts/lib/build-common.sh"

(($# == 2)) || die "usage: ${0##*/} TARGET PACKAGE"
target=$1
package=$2
load_target "${target}"
[[ -f "${package}" ]] || die "package not found: ${package}"

expected_iid="org.kde.kwin.EffectPluginFactory${TARGET_KWIN_UPSTREAM}"
if version_less_than "${TARGET_KWIN_UPSTREAM}" "6.7.0"; then
    expected_image_item_mode=renderer-factory
else
    expected_image_item_mode=unified
fi

if [[ "${TARGET_PACKAGE_KIND}" == debian ]]; then
    require_command podman
    package=$(realpath "${package}")
    image_tag="localhost/kwin-autoscroll-builder:${target}"
    podman image exists "${image_tag}" ||
        die "build the pinned ${target} image before verifying Debian packages"
    podman run --rm --network=none \
        --volume "${package}:/package.deb:ro" \
        "${image_tag}" bash -lc '
            set -euo pipefail
            expected_iid=$1
            multiarch=$2
            depends=$(dpkg-deb -f /package.deb Depends)
            grep -Eq "(^|,)[[:space:]]*kwin-wayland([[:space:]]*,|$)" <<<"${depends}"
            ! grep -Eq "(^|,)[[:space:]]*kwin-wayland[[:space:]]*\\(" <<<"${depends}"
            stage=$(mktemp -d)
            trap "rm -rf -- ${stage}" EXIT
            dpkg-deb -x /package.deb "${stage}"
            effect="${stage}/usr/lib/${multiarch}/qt6/plugins/kwin/effects/plugins/autoscroll.so"
            kcm="${stage}/usr/lib/${multiarch}/qt6/plugins/kwin/effects/configs/kwin_autoscroll_config.so"
            test -f "${effect}"
            test -f "${kcm}"
            actual_iids=$(strings -a "${effect}" |
                sed -n "s/^.*\\(org\\.kde\\.kwin\\.EffectPluginFactory[0-9][0-9.]*\\).*$/\\1/p" |
                sort -u)
            test "${actual_iids}" = "${expected_iid}"
            readelf -d "${effect}" | grep -q "Shared library: \\[libkwin\\.so\\.6\\]"
            symbols=$(nm -D -C "${effect}")
            case "$3" in
                renderer-factory)
                    grep -q "KWin::Scene::renderer() const" <<<"${symbols}"
                    ! grep -q "KWin::ImageItem::ImageItem(KWin::Item\\*)" <<<"${symbols}"
                    ;;
                unified)
                    grep -q "KWin::ImageItem::ImageItem(KWin::Item\\*)" <<<"${symbols}"
                    ;;
                *)
                    exit 2
                    ;;
            esac
        ' verify "${expected_iid}" \
        "${TARGET_MULTIARCH:?TARGET_MULTIARCH must be set for Debian targets}" \
        "${expected_image_item_mode}"
    printf 'Verified package metadata, paths, IID, and %s image items: %s\n' \
        "${expected_image_item_mode}" "${expected_iid}"
    exit 0
fi

require_command nm
stage=$(mktemp -d)
trap 'rm -rf -- "${stage}"' EXIT

case "${TARGET_PACKAGE_KIND}" in
    arch)
        bsdtar -xpf "${package}" -C "${stage}"
        package_info=$(bsdtar -xOf "${package}" .PKGINFO)
        grep -qx 'depend = kwin' <<<"${package_info}" ||
            die "package does not contain an unversioned kwin dependency"
        if grep -Eq '^depend = kwin[<>=]' <<<"${package_info}"; then
            die "package contains a version-constrained kwin dependency"
        fi
        effect="${stage}/usr/lib/qt6/plugins/kwin/effects/plugins/autoscroll.so"
        kcm="${stage}/usr/lib/qt6/plugins/kwin/effects/configs/kwin_autoscroll_config.so"
        ;;
    *)
        die "unsupported package kind: ${TARGET_PACKAGE_KIND}"
        ;;
esac

[[ -f "${effect}" ]] || die "effect plugin missing from expected path"
[[ -f "${kcm}" ]] || die "configuration plugin missing from expected path"
actual_iids=$(strings -a "${effect}" | sed -n 's/^.*\(org\.kde\.kwin\.EffectPluginFactory[0-9][0-9.]*\).*$/\1/p' | sort -u)
[[ "${actual_iids}" == "${expected_iid}" ]] ||
    die "embedded IID '${actual_iids}' does not match '${expected_iid}'"
readelf -d "${effect}" | grep -q 'Shared library: \[libkwin\.so\.6\]' ||
    die "effect does not link to libkwin.so.6"

symbols=$(nm -D -C "${effect}")
case "${expected_image_item_mode}" in
    renderer-factory)
        [[ "${symbols}" == *"KWin::Scene::renderer() const"* ]] ||
            die "legacy target does not use the scene renderer image-item factory"
        [[ "${symbols}" != *"KWin::ImageItem::ImageItem(KWin::Item*)"* ]] ||
            die "legacy target directly constructs the non-rendering base ImageItem"
        ;;
    unified)
        [[ "${symbols}" == *"KWin::ImageItem::ImageItem(KWin::Item*)"* ]] ||
            die "modern target does not construct the unified ImageItem"
        ;;
esac

printf 'Verified package metadata, paths, IID, and %s image items: %s\n' \
    "${expected_image_item_mode}" "${expected_iid}"
