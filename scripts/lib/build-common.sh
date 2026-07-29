#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || die "required command not found: $1"
}

version_less_than() {
    local left=$1
    local right=$2
    [[ "${left}" != "${right}" ]] &&
        [[ "$(printf '%s\n%s\n' "${left}" "${right}" | sort -V | head -n 1)" == "${left}" ]]
}

default_build_root() {
    local cache_base
    cache_base=${XDG_CACHE_HOME:-"${HOME}/.cache"}
    printf '%s\n' "${cache_base}/kwin-autoscroll-builds"
}

validate_build_root() {
    local requested=$1
    [[ -n "${requested}" ]] || die "build root must not be empty"
    [[ "${requested}" = /* ]] || die "build root must be an absolute path"
    [[ "${requested}" != / ]] || die "refusing to use / as a build root"
    [[ "${requested}" != "${HOME}" ]] || die "refusing to use the home directory as a build root"
    case "${requested}/" in
        "${project_root}/"*) die "build root must be outside the source repository" ;;
    esac
    mkdir -p -- "${requested}"
    readlink -f -- "${requested}"
}

load_target() {
    local target=$1
    local definition="${project_root}/build-envs/${target}/target.env"
    [[ -f "${definition}" ]] || die "unknown target '${target}'"
    # target.env files contain repository-controlled scalar assignments only.
    # shellcheck disable=SC1090
    source "${definition}"
}

print_target_versions() {
    printf '%s\n' "Target: ${TARGET_NAME}"
    printf '  distribution: %s %s (%s)\n' "${TARGET_DISTRIBUTION}" "${TARGET_RELEASE}" "${TARGET_ARCH}"
    printf '  KWin:         upstream %s; package %s\n' "${TARGET_KWIN_UPSTREAM}" "${TARGET_KWIN_PACKAGE}"
    printf '  plugin IID:   org.kde.kwin.EffectPluginFactory%s\n' "${TARGET_KWIN_UPSTREAM}"
    printf '  Qt:           %s\n' "${TARGET_QT}"
    printf '  KDE Frameworks: %s\n' "${TARGET_KF}"
    printf '  glibc:        %s\n' "${TARGET_GLIBC}"
    printf '  compiler:     %s\n' "${TARGET_COMPILER}"
}

project_version() {
    sed -n 's/^project(kwin-autoscroll VERSION \([^ ]*\).*/\1/p' "${project_root}/CMakeLists.txt"
}

create_source_archive() {
    local destination=$1
    local version epoch file_list
    version=$(project_version)
    epoch=${SOURCE_DATE_EPOCH:-$(git -C "${project_root}" log -1 --format=%ct)}
    file_list=$(mktemp)
    trap 'rm -f -- "${file_list}"' RETURN
    git -C "${project_root}" ls-files -z >"${file_list}"
    tar -C "${project_root}" \
        --null --files-from="${file_list}" \
        --sort=name \
        --mtime="@${epoch}" \
        --owner=0 --group=0 --numeric-owner \
        --mode='u+rwX,go+rX,go-w' \
        --transform="s,^,kwin-autoscroll-${version}/," \
        -cJf "${destination}"
}

source_state() {
    local commit dirty
    commit=$(git -C "${project_root}" rev-parse HEAD)
    if [[ -n "$(git -C "${project_root}" status --porcelain --untracked-files=no)" ]]; then
        dirty='+working-tree'
    else
        dirty=''
    fi
    printf '%s%s\n' "${commit}" "${dirty}"
}

write_checksum() {
    local artifact=$1
    sha256sum "${artifact}" >"${artifact}.sha256"
}
