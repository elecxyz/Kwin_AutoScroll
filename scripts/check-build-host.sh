#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# shellcheck source=scripts/lib/build-common.sh
source "${project_root}/scripts/lib/build-common.sh"

usage() {
    printf 'Usage: %s [TARGET...]\n' "${0##*/}"
    printf 'Targets: cachyos, steamos-6.4.3, kubuntu-26.04\n'
    printf 'With no targets, checks the host for the complete build matrix.\n'
}

targets=("$@")
if ((${#targets[@]} == 0)); then
    targets=(cachyos steamos-6.4.3 kubuntu-26.04)
fi

declare -A required=()
needs_arch=false
needs_podman=false

for target in "${targets[@]}"; do
    case "${target}" in
        -h|--help)
            usage
            exit 0
            ;;
    esac

    load_target "${target}"
    case "${TARGET_PACKAGE_KIND}" in
        arch)
            needs_arch=true
            for command in \
                arch-nspawn bsdtar cmp curl diff gpg makechrootpkg mkarchroot \
                namcap nm pacman-key readelf sha256sum sudo; do
                required["${command}"]=1
            done
            ;;
        debian)
            needs_podman=true
            for command in cmp diff podman sha256sum; do
                required["${command}"]=1
            done
            ;;
        *)
            die "unsupported package kind for ${target}: ${TARGET_PACKAGE_KIND}"
            ;;
    esac
done

failures=()
while IFS= read -r command; do
    if path=$(command -v "${command}" 2>/dev/null); then
        printf 'ok      %-18s %s\n' "${command}" "${path}"
    else
        printf 'missing %-18s\n' "${command}"
        failures+=("${command}")
    fi
done < <(printf '%s\n' "${!required[@]}" | sort)

if [[ "${needs_arch}" == true ]]; then
    # shellcheck source=/dev/null
    source /etc/os-release
    if [[ " ${ID:-} ${ID_LIKE:-} " != *" arch "* ]]; then
        printf 'error: Arch package targets require an Arch-family host with devtools\n' >&2
        failures+=("arch-family host")
    else
        printf 'ok      %-18s %s\n' "host" "${PRETTY_NAME:-${ID}}"
    fi

    if command -v sudo >/dev/null && ! sudo -n true 2>/dev/null; then
        printf 'note: sudo authentication will be requested by Arch target builds\n'
    fi
fi

if [[ "${needs_podman}" == true ]] && command -v podman >/dev/null; then
    if podman info >/dev/null 2>&1; then
        printf 'ok      %-18s rootless container storage is usable\n' "podman"
    else
        printf 'error: podman is installed but rootless container storage is unusable\n' >&2
        failures+=("podman runtime")
    fi
fi

if ((${#failures[@]})); then
    printf 'Host is not ready; failed checks: %s\n' "${failures[*]}" >&2
    exit 1
fi

printf 'Host is ready for: %s\n' "${targets[*]}"
