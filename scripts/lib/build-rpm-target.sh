#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

target=$1
build_root=$2
project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
# shellcheck source=scripts/lib/build-common.sh
source "${project_root}/scripts/lib/build-common.sh"
load_target "${target}"

for command in cmp diff podman sha256sum; do
    require_command "${command}"
done

target_dir="${build_root}/${target}"
workspace="${target_dir}/workspace"
artifacts="${target_dir}/artifacts"
reports="${target_dir}/reports"
dnf_cache="${target_dir}/dnf-cache"
topdir="${workspace}/rpmbuild"
mkdir -p -- "${artifacts}" "${reports}" "${dnf_cache}" \
    "${topdir}/BUILD" "${topdir}/BUILDROOT" "${topdir}/RPMS" \
    "${topdir}/SOURCES" "${topdir}/SPECS" "${topdir}/SRPMS"

image_tag="localhost/kwin-autoscroll-builder:${target}"
if ! podman image exists "${TARGET_BASE_IMAGE}"; then
    printf 'Pulling pinned base image: %s\n' "${TARGET_BASE_IMAGE}"
    podman pull "${TARGET_BASE_IMAGE}"
fi
podman build --pull=never --network=host --tag "${image_tag}" \
    --volume "${dnf_cache}:/var/cache/libdnf5:rw" \
    --file "${project_root}/build-envs/${target}/Containerfile" \
    "${project_root}/build-envs/${target}"
image_id=$(podman image inspect "${image_tag}" --format '{{.Id}}')

version=$(project_version)
export SOURCE_DATE_EPOCH=${SOURCE_DATE_EPOCH:-$(git -C "${project_root}" log -1 --format=%ct)}
archive="${topdir}/SOURCES/kwin-autoscroll-${version}.tar.xz"
create_source_archive "${archive}"
cp -f -- "${project_root}/packaging/fedora/kwin-autoscroll.rpmlintrc" \
    "${topdir}/SOURCES/kwin-autoscroll.rpmlintrc"
sed "s/@VERSION@/${version}/g" \
    "${project_root}/packaging/fedora/kwin-autoscroll.spec.in" \
    > "${topdir}/SPECS/kwin-autoscroll.spec"

find "${topdir}/RPMS" "${topdir}/SRPMS" -type f -name 'kwin-autoscroll-*.rpm' -delete
podman run --rm --network=none \
    --env "SOURCE_DATE_EPOCH=${SOURCE_DATE_EPOCH}" \
    --volume "${workspace}:/work:rw" \
    --volume "${project_root}/scripts/lib/build-rpm-in-container.sh:/runner.sh:ro" \
    "${image_tag}" /runner.sh /work/rpmbuild "${version}" \
    "${TARGET_RPM_RELEASE:?TARGET_RPM_RELEASE must be set for RPM targets}" \
    "${TARGET_KWIN_PACKAGE}" "${TARGET_QT_PACKAGE}" "${TARGET_KF_PACKAGE}" \
    "${TARGET_KWIN_UPSTREAM}" 2>&1 | tee "${reports}/build-report.txt"

built_package=$(find "${topdir}/RPMS" -type f \
    -name "kwin-autoscroll-${version}-${TARGET_RPM_RELEASE}.x86_64.rpm" -print -quit)
built_source_package=$(find "${topdir}/SRPMS" -type f \
    -name "kwin-autoscroll-${version}-${TARGET_RPM_RELEASE}.src.rpm" -print -quit)
[[ -n "${built_package}" ]] || die "rpmbuild did not produce the expected binary RPM"
[[ -n "${built_source_package}" ]] || die "rpmbuild did not produce the expected source RPM"

artifact="${artifacts}/$(basename "${built_package}")"
source_artifact="${artifacts}/$(basename "${built_source_package}")"
cp -f -- "${built_package}" "${artifact}"
cp -f -- "${built_source_package}" "${source_artifact}"
"${project_root}/scripts/verify-package.sh" "${target}" "${artifact}"
write_checksum "${artifact}"
write_checksum "${source_artifact}"

lock_file="${project_root}/build-envs/${target}/packages.lock.tsv"
candidate_lock="${reports}/packages.lock.candidate.tsv"
cp -f -- "${workspace}/installed-packages.tsv" "${candidate_lock}"
if [[ -s "${lock_file}" ]] && ! cmp -s "${lock_file}" "${candidate_lock}"; then
    diff -u "${lock_file}" "${candidate_lock}" >&2 || true
    die "resolved ${target} package set differs from ${lock_file}; create a new target definition instead of overwriting the lock"
fi
cp -f -- "${candidate_lock}" "${lock_file}"

{
    printf 'target=%s\n' "${target}"
    printf 'date=%s\n' "$(date --iso-8601=seconds)"
    printf 'source=%s\n' "$(source_state)"
    printf 'source_archive_sha256=%s\n' "$(sha256sum "${archive}" | awk '{print $1}')"
    printf 'base_image=%s\nbuilder_image=%s\n' "${TARGET_BASE_IMAGE}" "${image_id}"
    printf 'kwin_upstream=%s\nkwin_package=%s\n' "${TARGET_KWIN_UPSTREAM}" "${TARGET_KWIN_PACKAGE}"
    printf 'qt=%s\nkf=%s\nglibc=%s\ncompiler=%s\n' \
        "${TARGET_QT_PACKAGE}" "${TARGET_KF_PACKAGE}" "${TARGET_GLIBC}" "${TARGET_COMPILER}"
    printf 'plugin_iid=org.kde.kwin.EffectPluginFactory%s\n' "${TARGET_KWIN_UPSTREAM}"
    printf 'artifact=%s\nsha256=%s\n' "${artifact}" \
        "$(sha256sum "${artifact}" | awk '{print $1}')"
    printf 'source_artifact=%s\nsource_sha256=%s\n' "${source_artifact}" \
        "$(sha256sum "${source_artifact}" | awk '{print $1}')"
    printf 'status=build, tests, IID, linkage, contents, rpmlint, install/remove verified\n'
} > "${reports}/artifact-manifest.txt"

printf 'Artifact: %s\n' "${artifact}"
cat "${artifact}.sha256"
printf 'Source artifact: %s\n' "${source_artifact}"
cat "${source_artifact}.sha256"
