#!/usr/bin/env bash
# Tier 1.2: mk_s2sendump -pocketsphinx output loads in Python and PocketSphinx.
set -euo pipefail

root="$(cd "$(dirname "$0")/../.." && pwd)"
bindir="${SPHINXTRAIN_BIN_DIR:-${root}/build}"
hmm_res="${root}/test/res/hmm"
work="${root}/test/work/sendump_smoke"
hmm_dir="${work}/hmm"

mk_s2="${bindir}/mk_s2sendump"
mdef="${hmm_res}/RM.1000.mdef"
mixw="${hmm_res}/mixture_weights"

if [[ ! -x "${mk_s2}" ]]; then
    echo "skip: ${mk_s2} not built (cmake --build build)" >&2
    exit 0
fi
if [[ ! -f "${mdef}" || ! -f "${mixw}" ]]; then
    echo "test_sendump_pocketsphinx: missing ${hmm_res} fixtures" >&2
    exit 1
fi

rm -rf "${work}"
mkdir -p "${hmm_dir}"

echo "Running mk_s2sendump -pocketsphinx on test/res/hmm fixtures..."
"${mk_s2}" \
    -moddeffn "${mdef}" \
    -mixwfn "${mixw}" \
    -sendumpfn "${hmm_dir}/sendump" \
    -pocketsphinx yes

echo "Loading sendump with python/cmusphinx/sendump.py..."
python3 -c "
import sys
from pathlib import Path
sys.path.insert(0, '${root}/python')
from cmusphinx.sendump import Sendump
s = Sendump('${hmm_dir}/sendump')
if s.opdf.shape != (1147, 1, 2):
    raise SystemExit(f'unexpected sendump shape {s.opdf.shape}')
print('ok sendump.py loads PocketSphinx-format dump')
"

ps_batch="${POCKETSPHINX_BATCH:-}"
if [[ -z "${ps_batch}" ]]; then
    if [[ -x "${bindir}/pocketsphinx_batch" ]]; then
        ps_batch="${bindir}/pocketsphinx_batch"
    elif [[ -x "${root}/../pocketsphinx/build/pocketsphinx_batch" ]]; then
        ps_batch="${root}/../pocketsphinx/build/pocketsphinx_batch"
    fi
fi

if [[ -n "${ps_batch}" && -x "${ps_batch}" ]]; then
    cp "${mdef}" "${hmm_dir}/mdef"
    cp "${hmm_res}/means" "${hmm_dir}/means"
    cp "${hmm_res}/variances" "${hmm_dir}/variances"
    cp "${root}/etc/feat.params" "${hmm_dir}/feat.params"
    echo "Checking PocketSphinx loads HMM dir (sendump + mdef)..."
    ps_out="$("${ps_batch}" -hmm "${hmm_dir}" 2>&1)" || true
    if [[ "${ps_out}" == *"nothing to do in batch mode"* ]]; then
        echo "ok pocketsphinx_batch accepts sendump under -hmm"
    else
        echo "pocketsphinx_batch -hmm failed unexpectedly: ${ps_out}" >&2
        exit 1
    fi
else
    echo "skip pocketsphinx_batch (not in build/ or POCKETSPHINX_BATCH)"
fi

echo "test_sendump_pocketsphinx passed"
