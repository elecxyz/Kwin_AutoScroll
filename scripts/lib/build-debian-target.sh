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
apt_archives="${target_dir}/apt-package-cache/archives"
apt_lists="${target_dir}/apt-package-cache/lists"
mkdir -p -- "${workspace}" "${artifacts}" "${reports}" \
    "${apt_archives}/partial" "${apt_lists}/partial"

image_tag="localhost/kwin-autoscroll-builder:${target}"
podman build --pull=never --network=host --tag "${image_tag}" \
    --volume "${apt_archives}:/var/cache/apt/archives:rw" \
    --volume "${apt_lists}:/var/lib/apt/lists:rw" \
    --file "${project_root}/build-envs/${target}/Containerfile" \
    "${project_root}/build-envs/${target}"
image_id=$(podman image inspect "${image_tag}" --format '{{.Id}}')

version=$(project_version)
debian_version=$(sed -n '1s/^[^(]*(\([^)]*\)).*/\1/p' "${project_root}/debian/changelog")
[[ "${debian_version}" == "${version}-"* ]] ||
    die "Debian changelog version ${debian_version} does not match project ${version}"
archive="${workspace}/kwin-autoscroll-${version}.tar.xz"
create_source_archive "${archive}"
source_dir="${workspace}/kwin-autoscroll-${version}"
if [[ -d "${source_dir}" ]]; then
    find "${source_dir}" -mindepth 1 -maxdepth 1 -exec rm -rf -- {} +
else
    mkdir -p "${source_dir}"
fi
tar -xJf "${archive}" -C "${workspace}"
find "${workspace}" -maxdepth 1 -type f \( -name '*.deb' -o -name '*.changes' -o -name '*.buildinfo' \) -delete

container_script='
set -euo pipefail
cd /work/kwin-autoscroll-'"${version}"'
kwin_dev=$(dpkg-query -W -f="\${Version}" kwin-dev)
kwin_wayland=$(dpkg-query -W -f="\${Version}" kwin-wayland)
qt=$(dpkg-query -W -f="\${Version}" qt6-base-dev)
kf=$(dpkg-query -W -f="\${Version}" extra-cmake-modules)
printf "kwin-dev policy and installed versions:\n"
apt-cache policy kwin-dev kwin-wayland
test "${kwin_dev}" = "'"${TARGET_KWIN_PACKAGE}"'"
test "${kwin_wayland}" = "'"${TARGET_KWIN_PACKAGE}"'"
test "${qt}" = "'"${TARGET_QT_PACKAGE}"'"
test "${kf}" = "'"${TARGET_KF_PACKAGE}"'"
dpkg-buildpackage --build=binary --no-sign
package=$(find /work -maxdepth 1 -type f -name "kwin-autoscroll_'"${debian_version}"'_amd64.deb" -print -quit)
test -n "${package}"
lintian --display-info --pedantic "${package}"
depends=$(dpkg-deb -f "${package}" Depends)
case "${depends}" in
  *"kwin-wayland (= '"${TARGET_KWIN_PACKAGE}"')"*) ;;
  *) printf "bad Depends: %s\n" "${depends}" >&2; exit 1 ;;
esac
stage=$(mktemp -d)
trap "rm -rf -- ${stage}" EXIT
dpkg-deb -x "${package}" "${stage}"
effect="${stage}/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/plugins/autoscroll.so"
kcm="${stage}/usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/configs/kwin_autoscroll_config.so"
test -f "${effect}"
test -f "${kcm}"
test "$(strings -a "${effect}" | sed -n "s/^.*\\(org\\.kde\\.kwin\\.EffectPluginFactory[0-9][0-9.]*\\).*$/\\1/p" | sort -u)" = "org.kde.kwin.EffectPluginFactory'"${TARGET_KWIN_UPSTREAM}"'"
printf "\nPackage contents:\n"
dpkg-deb -c "${package}"
printf "\nELF metadata:\n"
readelf -d "${effect}" | grep NEEDED
readelf -d "${kcm}" | grep NEEDED
printf "\nDisposable package installation and target-root linkage:\n"
apt-get install -y "${package}"
effect_ldd=$(ldd /usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/plugins/autoscroll.so)
kcm_ldd=$(ldd /usr/lib/x86_64-linux-gnu/qt6/plugins/kwin/effects/configs/kwin_autoscroll_config.so)
printf "%s\n%s\n" "${effect_ldd}" "${kcm_ldd}"
case "${effect_ldd}${kcm_ldd}" in
  *"not found"*) printf "unresolved target-root shared library\n" >&2; exit 1 ;;
esac
apt-get remove -y kwin-autoscroll
if dpkg-query -W kwin-autoscroll >/dev/null 2>&1; then exit 1; fi
dpkg-query -W -f="\${Package}\t\${Version}\n" | sort > /work/installed-packages.tsv
'

podman run --rm --network=none \
    --volume "${workspace}:/work:rw" \
    "${image_tag}" bash -lc "${container_script}" 2>&1 |
    tee "${reports}/build-report.txt"

built_package="${workspace}/kwin-autoscroll_${debian_version}_amd64.deb"
[[ -f "${built_package}" ]] || die "Debian build did not produce the expected package"
artifact="${artifacts}/kwin-autoscroll_${debian_version}-${TARGET_ARTIFACT_LABEL}_amd64.deb"
cp -f -- "${built_package}" "${artifact}"
"${project_root}/scripts/verify-package.sh" "${target}" "${artifact}"
write_checksum "${artifact}"

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
    printf 'artifact=%s\n' "${artifact}"
    printf 'sha256=%s\n' "$(sha256sum "${artifact}" | awk '{print $1}')"
    printf 'status=build, tests, IID, linkage, contents, lintian, install/remove verified\n'
} >"${reports}/artifact-manifest.txt"

printf 'Artifact: %s\n' "${artifact}"
cat "${artifact}.sha256"
