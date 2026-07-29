#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 KWin AutoScroll contributors
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
args=("$@")
failures=()

for target in cachyos steamos-6.4.3 kubuntu-26.04; do
    printf '\n===== Building %s =====\n' "${target}"
    if ! "${project_root}/scripts/build-target.sh" "${args[@]}" "${target}"; then
        failures+=("${target}")
    fi
done

if ((${#failures[@]})); then
    printf 'Failed targets: %s\n' "${failures[*]}" >&2
    exit 1
fi
