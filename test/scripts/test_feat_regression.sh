#!/usr/bin/env bash
# Tier 2: regression for sphinx_fe output (guards libsphinxbase/fe cherry-picks).
set -euo pipefail

root="$(cd "$(dirname "$0")/../.." && pwd)"
bindir="${SPHINXTRAIN_BIN_DIR:-${root}/build}"
fixture="${root}/test/fixtures/feat_regression"
golden="${fixture}/utt0.mfc.sha256"
raw="${fixture}/utt0.raw"
work="${root}/test/work/feat_regression"
out_mfc="${work}/utt0.mfc"

sphinx_fe="${bindir}/sphinx_fe"

if [[ ! -x "${sphinx_fe}" ]]; then
    echo "skip: ${sphinx_fe} not built (cmake --build build)" >&2
    exit 0
fi
if [[ ! -f "${raw}" || ! -f "${golden}" ]]; then
    echo "test_feat_regression: missing ${fixture} files" >&2
    exit 1
fi

rm -rf "${work}"
mkdir -p "${work}"

echo "Running sphinx_fe on feat_regression fixture (template etc/sphinx_train.cfg FE settings)..."
"${sphinx_fe}" \
    -i "${raw}" \
    -o "${out_mfc}" \
    -raw yes \
    -input_endian little \
    -samprate 16000 \
    -lowerf 130 \
    -upperf 6800 \
    -nfilt 25 \
    -nfft 512 \
    -transform dct \
    -lifter 22 \
    -ncep 13

got="$(shasum -a 256 "${out_mfc}" | awk '{print $1}')"
want="$(tr -d '[:space:]' < "${golden}")"

if [[ "${got}" == "${want}" ]]; then
    echo "ok feat regression mfc sha256 ${got}"
    exit 0
fi

if [[ "${UPDATE_GOLDEN:-}" == "1" ]]; then
    echo "${got}" > "${golden}"
    echo "updated ${golden}"
    exit 0
fi

echo "feat regression mismatch" >&2
echo "  got:  ${got}" >&2
echo "  want: ${want}" >&2
echo "  run UPDATE_GOLDEN=1 $0 to accept new output after an intentional FE change" >&2
exit 1
