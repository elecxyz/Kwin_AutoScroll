#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# shellcheck source=scripts/lib/build-common.sh
source "${project_root}/scripts/lib/build-common.sh"

usage() {
    printf 'Usage: %s [--root ABSOLUTE_PATH] TARGET\n' "${0##*/}"
    printf 'Targets: cachyos, steamos-6.4.3, kubuntu-26.04\n'
}

build_root=$(default_build_root)
target=
while (($#)); do
    case "$1" in
        --root)
            (($# >= 2)) || die "--root requires a path"
            build_root=$2
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        -*)
            die "unknown option: $1"
            ;;
        *)
            [[ -z "${target}" ]] || die "only one target may be specified"
            target=$1
            shift
            ;;
    esac
done

[[ -n "${target}" ]] || {
    usage >&2
    exit 2
}
load_target "${target}"
build_root=$(validate_build_root "${build_root}")
print_target_versions
printf '  external build root: %s\n' "${build_root}"

case "${TARGET_PACKAGE_KIND}" in
    arch)
        exec "${project_root}/scripts/lib/build-arch-target.sh" "${target}" "${build_root}"
        ;;
    debian)
        exec "${project_root}/scripts/lib/build-debian-target.sh" "${target}" "${build_root}"
        ;;
    *)
        die "unsupported package kind: ${TARGET_PACKAGE_KIND}"
        ;;
esac
