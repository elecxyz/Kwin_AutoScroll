#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
release_build="${project_root}/build-release"
dist_dir="${project_root}/dist"
version=$(sed -n 's/^project(kwin-autoscroll VERSION \([^ ]*\).*/\1/p' \
    "${project_root}/CMakeLists.txt")
[[ -n "${version}" ]] || {
    printf 'Unable to read the project version\n' >&2
    exit 1
}
kwin_version=$(sed -n 's/^#define KWIN_PLUGIN_VERSION_STRING "\(.*\)"/\1/p' /usr/include/kwin/config-kwin.h)

cmake -S "${project_root}" -B "${release_build}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "${release_build}"
ctest --test-dir "${release_build}" --output-on-failure
cmake --build "${release_build}" --target package_source

mkdir -p "${dist_dir}"
archive="${dist_dir}/kwin-autoscroll-${version}.tar.xz"
source_archive="${release_build}/kwin-autoscroll-${version}.tar.xz"
source_date_epoch="${SOURCE_DATE_EPOCH:-$(git -C "${project_root}" log -1 --format=%ct)}"
archive_stage=$(mktemp -d "${release_build}/source-stage.XXXXXX")
trap 'rm -rf -- "${archive_stage}"' EXIT

tar -xJf "${source_archive}" -C "${archive_stage}"
tar \
    --sort=name \
    --mtime="@${source_date_epoch}" \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --mode='u+rwX,go+rX,go-w' \
    -C "${archive_stage}" \
    -cJf "${archive}" \
    "kwin-autoscroll-${version}"
checksum=$(sha256sum "${archive}" | cut -d' ' -f1)

sed \
    -e "s/@VERSION@/${version}/g" \
    -e "s/@KWIN_VERSION@/${kwin_version}/g" \
    -e "s/@SHA256@/${checksum}/g" \
    "${project_root}/packaging/arch/PKGBUILD.in" \
    > "${dist_dir}/PKGBUILD"

printf 'Created %s\nCreated %s\n' "${archive}" "${dist_dir}/PKGBUILD"
