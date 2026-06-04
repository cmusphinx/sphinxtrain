#!/usr/bin/env bash
# Verify a PocketSphinx tree includes the two-pass alignment fix (cmusphinx/pocketsphinx#468).
# Until upstream merges #468, apply test/scripts/apply_pocketsphinx_align_patch.sh first.
# Exit 0 if the fix is present; exit 1 if the tree is too old; exit 2 on usage errors.
set -euo pipefail

ps_root="${POCKETSPHINX_SRC:-}"
if [[ -z "${ps_root}" ]]; then
    if [[ -d ../pocketsphinx/src ]]; then
        ps_root="../pocketsphinx"
    elif [[ -d ../../pocketsphinx/src ]]; then
        ps_root="../../pocketsphinx"
    else
        echo "check_pocketsphinx_align_fix: set POCKETSPHINX_SRC to a pocketsphinx checkout" >&2
        exit 2
    fi
fi

src="${ps_root}/src/state_align_search.c"
if [[ ! -f "${src}" ]]; then
    echo "check_pocketsphinx_align_fix: missing ${src}" >&2
    exit 2
fi

if grep -q 'nf >= sas->ef' "${src}" \
    && grep -q 'hmm_frame(hmm) < frame_idx' "${src}"; then
    echo "ok pocketsphinx includes two-pass alignment fix (#468)"
    exit 0
fi

echo "check_pocketsphinx_align_fix: ${src} lacks PR 468 (still on pre-fix prune/transition logic)" >&2
exit 1
