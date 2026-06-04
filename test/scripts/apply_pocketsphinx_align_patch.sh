#!/usr/bin/env bash
# Apply pocketsphinx#468 to a checkout until it is merged upstream.
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/../.." && pwd)"
patch_file="${repo_root}/test/patches/pocketsphinx-468-state_align_search.patch"

ps_root="${1:-${POCKETSPHINX_SRC:-}}"
if [[ -z "${ps_root}" ]]; then
    if [[ -d "${repo_root}/../pocketsphinx/src" ]]; then
        ps_root="${repo_root}/../pocketsphinx"
    else
        echo "apply_pocketsphinx_align_patch: pass pocketsphinx root or set POCKETSPHINX_SRC" >&2
        exit 2
    fi
fi

src="${ps_root}/src/state_align_search.c"
if [[ ! -f "${src}" ]]; then
    echo "apply_pocketsphinx_align_patch: missing ${src}" >&2
    exit 2
fi
if [[ ! -f "${patch_file}" ]]; then
    echo "apply_pocketsphinx_align_patch: missing ${patch_file}" >&2
    exit 2
fi

if grep -q 'nf >= sas->ef' "${src}" \
    && grep -q 'hmm_frame(hmm) < frame_idx' "${src}"; then
    echo "ok pocketsphinx align patch already present (#468)"
    exit 0
fi

echo "Applying pocketsphinx#468 patch to ${ps_root}..."
patch -p1 -d "${ps_root}" < "${patch_file}"
echo "ok applied ${patch_file}"
