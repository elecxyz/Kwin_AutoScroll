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

for command in arch-nspawn bsdtar cmp curl diff gpg makechrootpkg mkarchroot namcap pacman-key readelf sha256sum sudo; do
    require_command "${command}"
done

target_dir="${build_root}/${target}"
root_dir="${target_dir}/chroot"
package_cache="${target_dir}/package-cache"
workspace="${target_dir}/workspace"
artifacts="${target_dir}/artifacts"
reports="${target_dir}/reports"
mkdir -p -- "${root_dir}" "${package_cache}" "${workspace}" "${artifacts}" "${reports}"

pacman_config="${project_root}/build-envs/${target}/pacman.conf"
makepkg_config="${project_root}/build-envs/${target}/makepkg.conf"

if [[ "${target}" == steamos-* ]]; then
    keyring_dir="${target_dir}/holo-keyring"
    keyring_archive="${keyring_dir}/holo-keyring-${TARGET_KEYRING_VERSION}-any.pkg.tar.zst"
    keyring_url="https://steamdeck-packages.steamos.cloud/archlinux-mirror/holo-main/os/x86_64/$(basename "${keyring_archive}")"
    mkdir -p -- "${keyring_dir}/files" "${target_dir}/pacman-gnupg"
    if [[ ! -f "${keyring_archive}" ]]; then
        curl --proto '=https' --tlsv1.2 --fail --location --show-error \
            "${keyring_url}" -o "${keyring_archive}"
    fi
    printf '%s  %s\n' "${TARGET_KEYRING_SHA256}" "${keyring_archive}" | sha256sum -c -
    bsdtar -xpf "${keyring_archive}" -C "${keyring_dir}/files"
    gpg --show-keys --with-colons "${keyring_dir}/files/usr/share/pacman/keyrings/holo.gpg" |
        grep -q "^fpr:::::::::${TARGET_SIGNING_FINGERPRINT}:$" ||
        die "Valve signing fingerprint is absent from the pinned holo-keyring"
    if [[ ! -s "${target_dir}/pacman-gnupg/pubring.gpg" &&
          ! -s "${target_dir}/pacman-gnupg/pubring.kbx" ]]; then
        sudo pacman-key --gpgdir "${target_dir}/pacman-gnupg" --init
        sudo pacman-key --gpgdir "${target_dir}/pacman-gnupg" --populate archlinux
        sudo pacman-key --gpgdir "${target_dir}/pacman-gnupg" \
            --populate-from "${keyring_dir}/files/usr/share/pacman/keyrings" --populate holo
    fi
    bootstrap_config="${target_dir}/pacman-bootstrap.conf"
    sed "/^\\[options\\]/a GPGDir = ${target_dir}/pacman-gnupg" \
        "${pacman_config}" >"${bootstrap_config}"
else
    bootstrap_config="${pacman_config}"
fi

if [[ ! -f "${root_dir}/root/.arch-chroot" ]]; then
    if [[ -e "${root_dir}/root" ]]; then
        incomplete_root="${root_dir}/root.incomplete-$(date -u +%Y%m%dT%H%M%SZ)"
        sudo mv -- "${root_dir}/root" "${incomplete_root}"
        printf 'Preserved incomplete chroot as: %s\n' "${incomplete_root}"
    fi
    sudo mkarchroot -C "${bootstrap_config}" -M "${makepkg_config}" \
        -c "${package_cache}" "${root_dir}/root" \
        appstream base-devel extra-cmake-modules kwin namcap ninja vulkan-headers
fi

if [[ "${target}" == steamos-* ]]; then
    sudo install -Dm644 "${pacman_config}" "${root_dir}/root/etc/pacman.conf"
    sudo install -Dm644 "${keyring_dir}/files/usr/share/pacman/keyrings/holo.gpg" \
        "${root_dir}/root/usr/share/pacman/keyrings/holo.gpg"
    sudo install -Dm644 "${keyring_dir}/files/usr/share/pacman/keyrings/holo-trusted" \
        "${root_dir}/root/usr/share/pacman/keyrings/holo-trusted"
    sudo install -Dm644 "${keyring_dir}/files/usr/share/pacman/keyrings/holo-revoked" \
        "${root_dir}/root/usr/share/pacman/keyrings/holo-revoked"
    sudo arch-nspawn "${root_dir}/root" pacman-key --populate holo
fi

sudo arch-nspawn -c "${package_cache}" "${root_dir}/root" pacman -S --needed --noconfirm \
    appstream ninja vulkan-headers

if [[ "${target}" == steamos-* ]]; then
    # Older roots may have obtained build-only dependencies through
    # arch-nspawn's default host cache. Migrate only exact packages present in
    # the signed Valve repository databases and accepted by the target
    # keyring. Fresh roots download directly to package_cache through -c.
    declare -A steam_repository_files=()
    for candidate in jupiter-main holo-main core-main extra-main community-main; do
        database="${root_dir}/root/var/lib/pacman/sync/${candidate}.db"
        [[ -f "${database}" ]] || die "SteamOS repository database missing: ${database}"
        if ! sudo bsdtar -tf "${database}" | awk '
            /\/desc$/ { found = 1 }
            END { exit found ? 0 : 1 }
        '; then
            continue
        fi
        while IFS= read -r filename; do
            steam_repository_files["${filename}"]=${candidate}
        done < <(
            sudo bsdtar -xOf "${database}" '*/desc' |
                awk '$0 == "%FILENAME%" { getline; print }'
        )
    done

    declare -A installed_versions=()
    while read -r name installed_version; do
        installed_versions["${name}"]=${installed_version}
    done < <(sudo arch-nspawn "${root_dir}/root" pacman -Q)

    for host_package in /var/cache/pacman/pkg/*.pkg.tar.*; do
        [[ -f "${host_package}" && "${host_package}" != *.sig ]] || continue
        filename=$(basename "${host_package}")
        [[ -n "${steam_repository_files["${filename}"]:-}" ]] || continue
        [[ ! -f "${package_cache}/${filename}" ]] || continue
        signature="${host_package}.sig"
        [[ -f "${signature}" ]] || continue
        name=$(bsdtar -xOf "${host_package}" .PKGINFO |
            sed -n 's/^pkgname = //p' | head -1)
        package_version=$(bsdtar -xOf "${host_package}" .PKGINFO |
            sed -n 's/^pkgver = //p' | head -1)
        [[ "${installed_versions["${name}"]:-}" == "${package_version}" ]] || continue
        sudo pacman-key --gpgdir "${root_dir}/root/etc/pacman.d/gnupg" \
            --verify "${signature}" "${host_package}" >/dev/null
        sudo cp -n -- "${host_package}" "${signature}" "${package_cache}/"
    done
fi

root_kwin=$(sudo arch-nspawn "${root_dir}/root" pacman -Q kwin | awk '{print $2}')
header_kwin=$(sudo arch-nspawn "${root_dir}/root" \
    sed -n 's/^#define KWIN_PLUGIN_VERSION_STRING "\(.*\)"/\1/p' /usr/include/kwin/config-kwin.h)
root_qt=$(sudo arch-nspawn "${root_dir}/root" pacman -Q qt6-base | awk '{print $2}' | cut -d- -f1)
root_kf=$(sudo arch-nspawn "${root_dir}/root" pacman -Q extra-cmake-modules | awk '{print $2}' | cut -d- -f1)
root_glibc=$(sudo arch-nspawn "${root_dir}/root" pacman -Q glibc | awk '{print $2}')
root_compiler=$(sudo arch-nspawn "${root_dir}/root" pacman -Q gcc | awk '{print $2}')
[[ "${root_kwin}" == "${TARGET_KWIN_PACKAGE}" ]] ||
    die "build root KWin ${root_kwin} differs from target ${TARGET_KWIN_PACKAGE}"
[[ "${header_kwin}" == "${TARGET_KWIN_UPSTREAM}" ]] ||
    die "KWin header IID ${header_kwin} differs from target ${TARGET_KWIN_UPSTREAM}"
[[ "${root_qt}" == "${TARGET_QT}" ]] ||
    die "build root Qt ${root_qt} differs from target ${TARGET_QT}"
[[ "${root_kf}" == "${TARGET_KF}" ]] ||
    die "build root KF ${root_kf} differs from target ${TARGET_KF}"
[[ "${root_glibc}" == "${TARGET_GLIBC}" ]] ||
    die "build root glibc ${root_glibc} differs from target ${TARGET_GLIBC}"
[[ "${root_compiler}" == "${TARGET_COMPILER}" ]] ||
    die "build root GCC ${root_compiler} differs from target ${TARGET_COMPILER}"

version=$(project_version)
export SOURCE_DATE_EPOCH=${SOURCE_DATE_EPOCH:-$(git -C "${project_root}" log -1 --format=%ct)}
archive="${workspace}/kwin-autoscroll-${version}.tar.xz"
create_source_archive "${archive}"
archive_sha=$(sha256sum "${archive}" | awk '{print $1}')
sed -e "s/@VERSION@/${version}/g" \
    -e "s/@KWIN_VERSION@/${root_kwin}/g" \
    -e "s/@SHA256@/${archive_sha}/g" \
    "${project_root}/packaging/arch/PKGBUILD.in" >"${workspace}/PKGBUILD"
package_release=$(sed -n 's/^pkgrel=//p' "${workspace}/PKGBUILD")
[[ "${package_release}" =~ ^[1-9][0-9]*$ ]] ||
    die "invalid pkgrel in generated PKGBUILD: ${package_release}"

find "${workspace}" -maxdepth 1 -type f -name 'kwin-autoscroll-*.pkg.tar.*' -delete
(
    cd "${workspace}"
    makechrootpkg -c -n -r "${root_dir}" -- --check --cleanbuild
)

for chroot_cache in \
    "${root_dir}/root/var/cache/pacman/pkg" \
    "${root_dir}/${USER}/var/cache/pacman/pkg"; do
    [[ -d "${chroot_cache}" ]] || continue
    sudo find "${chroot_cache}" -maxdepth 1 -type f \
        \( -name '*.pkg.tar.*' -o -name '*.pkg.tar.*.sig' \) \
        -exec cp -n -- {} "${package_cache}/" \;
done

built_package=$(find "${workspace}" -maxdepth 1 -type f \
    -name "kwin-autoscroll-${version}-${package_release}-*.pkg.tar.zst" \
    ! -name '*debug*' -print -quit)
[[ -n "${built_package}" ]] || die "makechrootpkg did not produce the expected package"
artifact="${artifacts}/kwin-autoscroll-${version}-${package_release}-${TARGET_ARTIFACT_LABEL}-x86_64.pkg.tar.zst"
cp -f -- "${built_package}" "${artifact}"
"${project_root}/scripts/verify-package.sh" "${target}" "${artifact}"

working_copy="${root_dir}/${USER}"
[[ -d "${working_copy}" ]] || die "makechrootpkg working copy not found: ${working_copy}"
verify_name=$(basename "${artifact}")
sudo cp "${artifact}" "${working_copy}/root/${verify_name}"
{
    printf 'Target package versions:\n'
    sudo arch-nspawn "${working_copy}" pacman -Q kwin qt6-base extra-cmake-modules glibc gcc cmake
    printf '\nPackage contents:\n'
    bsdtar -tf "${artifact}"
    printf '\nELF dependencies resolved inside target root:\n'
    sudo arch-nspawn "${working_copy}" pacman -U --noconfirm "/root/${verify_name}"
    effect_ldd=$(sudo arch-nspawn "${working_copy}" \
        ldd /usr/lib/qt6/plugins/kwin/effects/plugins/autoscroll.so)
    kcm_ldd=$(sudo arch-nspawn "${working_copy}" \
        ldd /usr/lib/qt6/plugins/kwin/effects/configs/kwin_autoscroll_config.so)
    printf '%s\n%s\n' "${effect_ldd}" "${kcm_ldd}"
    [[ "${effect_ldd}${kcm_ldd}" != *"not found"* ]] ||
        die "package has an unresolved shared-library dependency in the target root"
    printf '\nnamcap:\n'
    namcap "${artifact}"
    sudo arch-nspawn "${working_copy}" pacman -R --noconfirm kwin-autoscroll
    if sudo arch-nspawn "${working_copy}" pacman -Q kwin-autoscroll >/dev/null 2>&1; then
        die "package removal verification failed"
    fi
} | tee "${reports}/build-report.txt"

write_checksum "${artifact}"
source_archive_sha=$(sha256sum "${archive}" | awk '{print $1}')
sudo sha256sum "${root_dir}/root"/var/lib/pacman/sync/*.db |
    sed "s#${root_dir}/root/##" >"${reports}/repository-databases.sha256"
{
    printf 'target=%s\n' "${target}"
    printf 'date=%s\n' "$(date --iso-8601=seconds)"
    printf 'source=%s\n' "$(source_state)"
    printf 'source_archive_sha256=%s\n' "${source_archive_sha}"
    printf 'kwin_upstream=%s\nkwin_package=%s\n' "${TARGET_KWIN_UPSTREAM}" "${TARGET_KWIN_PACKAGE}"
    printf 'qt=%s\nkf=%s\nglibc=%s\ncompiler=%s\n' \
        "${TARGET_QT}" "${TARGET_KF}" "${TARGET_GLIBC}" "${TARGET_COMPILER}"
    printf 'plugin_iid=org.kde.kwin.EffectPluginFactory%s\n' "${TARGET_KWIN_UPSTREAM}"
    printf 'build_root=%s\n' "${root_dir}/root"
    printf 'repository_databases=%s\n' "${reports}/repository-databases.sha256"
    printf 'artifact=%s\n' "${artifact}"
    printf 'sha256=%s\n' "$(sha256sum "${artifact}" | awk '{print $1}')"
    printf 'status=build, tests, IID, linkage, contents, namcap, install/remove verified\n'
} >"${reports}/artifact-manifest.txt"

declare -A cached_versions=()
for package in "${package_cache}"/*.pkg.tar.*; do
    [[ -f "${package}" && "${package}" != *.sig ]] || continue
    name=$(bsdtar -xOf "${package}" .PKGINFO |
        sed -n 's/^pkgname = //p' | head -1)
    package_version=$(bsdtar -xOf "${package}" .PKGINFO |
        sed -n 's/^pkgver = //p' | head -1)
    cached_versions["${name}"]=${package_version}
done
while read -r name installed_version; do
    [[ "${cached_versions["${name}"]:-}" == "${installed_version}" ]] ||
        die "${target} input package is absent from the external cache: ${name} ${installed_version}"
done < <(sudo arch-nspawn "${root_dir}/root" pacman -Q)

if [[ "${target}" == steamos-* ]]; then
    lock_file="${project_root}/build-envs/${target}/packages.lock.tsv"
    {
        printf '# package\tversion\trepository\turl\tsha256\n'
        printf 'holo-keyring\t%s\tholo-main\t%s\t%s\n' \
            "${TARGET_KEYRING_VERSION}" "${keyring_url}" "${TARGET_KEYRING_SHA256}"
        for package in "${package_cache}"/*.pkg.tar.*; do
            [[ "${package}" != *.sig ]] || continue
            name=$(bsdtar -xOf "${package}" .PKGINFO | sed -n 's/^pkgname = //p' | head -1)
            package_version=$(bsdtar -xOf "${package}" .PKGINFO | sed -n 's/^pkgver = //p' | head -1)
            filename=$(basename "${package}")
            repo=${steam_repository_files["${filename}"]:-}
            [[ -n "${repo}" ]] || die "cannot map cached SteamOS package to a signed repository: ${filename}"
            url="https://steamdeck-packages.steamos.cloud/archlinux-mirror/${repo}/os/x86_64/${filename}"
            printf '%s\t%s\t%s\t%s\t%s\n' "${name}" "${package_version}" "${repo}" "${url}" \
                "$(sha256sum "${package}" | awk '{print $1}')"
        done | sort -u
    } >"${lock_file}"
else
    lock_file="${project_root}/build-envs/${target}/packages.lock.tsv"
    candidate_lock="${reports}/packages.lock.candidate.tsv"
    {
        printf '# package\tversion\n'
        sudo arch-nspawn "${root_dir}/root" pacman -Q | sort |
            awk '{ print $1 "\t" $2 }'
    } >"${candidate_lock}"
    if [[ -s "${lock_file}" ]] && ! cmp -s "${lock_file}" "${candidate_lock}"; then
        diff -u "${lock_file}" "${candidate_lock}" >&2 || true
        die "resolved ${target} package set differs from ${lock_file}; create a new target definition instead of overwriting the lock"
    fi
    cp -f -- "${candidate_lock}" "${lock_file}"
fi

printf 'Artifact: %s\n' "${artifact}"
cat "${artifact}.sha256"
