#!/usr/bin/env bash
# Tier 3.1: compare sphinx3_align vs pocketsphinx align on one AN4 (or export) utterance.
# Manual spike only — not run in CI until parity is understood.
#
# Prerequisites:
#   - Trained export with etc/sphinx_train.resolved.json (e.g. sibling an4 after sphinxtrain run)
#   - build/sphinx3_align, build/mk_s2sendump
#   - pocketsphinx with #468 patch (apply_pocketsphinx_align_patch.sh on POCKETSPHINX_SRC)
#
# Usage:
#   SPIKE_EXPORT_ROOT=/path/to/an4 test/scripts/spike_pocketsphinx_align.sh
#   SPIKE_UTT_ID=an406-fcaw-b SPIKE_HMM_DIR=.../an4.cd_cont_200  # overrides
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/../.." && pwd)"
bindir="${SPHINXTRAIN_BIN_DIR:-${repo_root}/build}"
work="${repo_root}/test/work/align_spike"
resolved=""

export_root="${SPIKE_EXPORT_ROOT:-}"
if [[ -z "${export_root}" ]]; then
    if [[ -d "${repo_root}/../an4/etc" ]]; then
        export_root="${repo_root}/../an4"
    fi
fi
if [[ -n "${export_root}" ]]; then
    resolved="${export_root}/etc/sphinx_train.resolved.json"
fi

if [[ ! -f "${resolved}" ]]; then
    echo "spike_pocketsphinx_align: set SPIKE_EXPORT_ROOT to a trained project" >&2
    echo "  (needs etc/sphinx_train.resolved.json, feat/, wav/, model_parameters/)" >&2
    exit 2
fi

read_cfg() {
    python3 - "$resolved" "$1" <<'PY'
import json, sys
data = json.load(open(sys.argv[1], encoding="utf-8"))
key = sys.argv[2]
d = data.get("derived", {})
v = data.get("variables", {})
if key in d:
    print(d[key])
elif key in v:
    print(v[key])
else:
    sys.exit(1)
PY
}

hmm_dir="${SPIKE_HMM_DIR:-}"
if [[ -z "${hmm_dir}" ]]; then
    hmm_dir="$(read_cfg cd_hmm_dir)" || {
        echo "spike_pocketsphinx_align: cd_hmm_dir missing in resolved config" >&2
        exit 2
    }
fi
dict="${SPIKE_DICT:-$(read_cfg dictionary)}"
filler="${SPIKE_FILLER:-$(read_cfg CFG_FILLERDICT)}"
feat_dir="${SPIKE_FEAT_DIR:-$(read_cfg CFG_FEATFILES_DIR)}"
wav_dir="${SPIKE_WAV_DIR:-$(read_cfg CFG_WAVFILES_DIR)}"
feat_ext="$(read_cfg CFG_FEATFILE_EXTENSION)"
feat_type="$(read_cfg CFG_FEATURE)"
ceplen="$(read_cfg CFG_VECTOR_LENGTH)"
agc="$(read_cfg CFG_AGC)"
cmn="$(read_cfg CFG_CMN)"
varnorm="$(read_cfg CFG_VARNORM)"
test_ctl="${SPIKE_CTL:-$(read_cfg test_listoffiles)}"
test_trans="${SPIKE_TRANSCRIPT:-$(read_cfg DEC_CFG_TRANSCRIPTFILE 2>/dev/null || true)}"
if [[ -z "${test_trans}" || ! -f "${test_trans}" ]]; then
    db="$(read_cfg CFG_DB_NAME)"
    test_trans="${export_root}/etc/${db}_test.transcription"
fi

utt_id="${SPIKE_UTT_ID:-}"
fileid="${SPIKE_FILEID:-}"
if [[ -z "${fileid}" ]]; then
    fileid="$(sed -n '1p' "${test_ctl}" | tr -d '\r')"
fi
if [[ -z "${utt_id}" ]]; then
    utt_id="$(basename "${fileid}")"
fi

trans_line="$(python3 - "${test_trans}" "${utt_id}" <<'PY'
import re, sys
path, utt = sys.argv[1:3]
for line in open(path, encoding="utf-8", errors="replace"):
    line = line.strip()
    if not line:
        continue
    m = re.search(r"\(([^)]+)\)\s*$", line)
    if not m or m.group(1) != utt:
        continue
    print(re.sub(r"\s*\([^)]+\)\s*$", "", line).strip())
    break
PY
)"
if [[ -z "${trans_line}" ]]; then
    echo "spike_pocketsphinx_align: no transcript line for ${utt_id} in ${test_trans}" >&2
    exit 2
fi

ps_words="${trans_line}"
insent="<s> ${trans_line} </s> (${utt_id})"
wav="${wav_dir}/${fileid}.wav"
mfc="${feat_dir}/${fileid}.${feat_ext}"

align_bin="${SPHINX3_ALIGN:-${bindir}/sphinx3_align}"
mk_s2="${bindir}/mk_s2sendump"
ps_bin="${POCKETSPHINX:-}"
if [[ -z "${ps_bin}" ]]; then
    if [[ -x "${bindir}/pocketsphinx" ]]; then
        ps_bin="${bindir}/pocketsphinx"
    elif [[ -x "${repo_root}/../pocketsphinx/build/pocketsphinx" ]]; then
        ps_bin="${repo_root}/../pocketsphinx/build/pocketsphinx"
    fi
fi

for need in "${align_bin}" "${mk_s2}" "${hmm_dir}/mdef" "${dict}" "${wav}" "${mfc}"; do
    if [[ ! -e "${need}" ]]; then
        echo "spike_pocketsphinx_align: missing ${need}" >&2
        exit 2
    fi
done
if [[ -z "${ps_bin}" || ! -x "${ps_bin}" ]]; then
    echo "spike_pocketsphinx_align: pocketsphinx binary not found (build PS or set POCKETSPHINX)" >&2
    exit 2
fi

if [[ -n "${POCKETSPHINX_SRC:-}" ]]; then
    "${repo_root}/test/scripts/apply_pocketsphinx_align_patch.sh" "${POCKETSPHINX_SRC}" >/dev/null
elif [[ -d "${repo_root}/../pocketsphinx/src" ]]; then
    "${repo_root}/test/scripts/apply_pocketsphinx_align_patch.sh" "${repo_root}/../pocketsphinx" >/dev/null \
        || true
fi

echo "Spike align: export=${export_root}"
echo "  hmm=${hmm_dir}"
echo "  utt=${utt_id} fileid=${fileid}"
echo "  ref words: ${trans_line}"

rm -rf "${work}"
mkdir -p "${work}/hmm_ps" "${work}/out"
hmm_ps="${work}/hmm_ps"

echo "Building PocketSphinx HMM dir (sendump)..."
"${mk_s2}" \
    -moddeffn "${hmm_dir}/mdef" \
    -mixwfn "${hmm_dir}/mixture_weights" \
    -sendumpfn "${hmm_ps}/sendump" \
    -pocketsphinx yes
cp "${hmm_dir}/mdef" "${hmm_dir}/means" "${hmm_dir}/variances" \
    "${hmm_dir}/mixture_weights" "${hmm_dir}/transition_matrices" \
    "${hmm_dir}/feat.params" "${hmm_ps}/"

printf '%s\n' "${fileid}" > "${work}/ctl"
printf '%s\n' "${insent}" > "${work}/insent"

echo "Running sphinx3_align..."
"${align_bin}" \
    -hmm "${hmm_dir}" \
    -senmgau .cont. \
    -mixwfloor 1e-8 \
    -varfloor 1e-4 \
    -dict "${dict}" \
    -fdict "${filler}" \
    -ctl "${work}/ctl" \
    -ctloffset 0 \
    -ctlcount 1 \
    -cepdir "${feat_dir}" \
    -cepext ".${feat_ext}" \
    -insent "${work}/insent" \
    -outsent "${work}/s3.outsent" \
    -beam 1e-308 \
    -agc "${agc}" \
    -cmn "${cmn}" \
    -varnorm "${varnorm}" \
    -feat "${feat_type}" \
    -ceplen "${ceplen}" \
    -insert_sil 1 \
    > "${work}/s3.log" 2>&1

echo "Running pocketsphinx align (-state_align yes)..."
"${ps_bin}" -loglevel INFO \
    -hmm "${hmm_ps}" \
    -dict "${dict}" \
    -state_align yes \
    align "${wav}" "${ps_words}" \
    > "${work}/ps.state.json" 2> "${work}/ps.log"

python3 - "${work}/s3.outsent" "${work}/ps.state.json" "${trans_line}" <<'PY'
import json, re, sys

outsent_path, ps_json_path, ref = sys.argv[1:4]
ref_words = ref.split()

s3_text = open(outsent_path, encoding="utf-8", errors="replace").read().strip()
words = []
for tok in s3_text.split():
    if tok in ("<s>", "</s>") or tok.startswith("("):
        continue
    if tok == "<sil>":
        words.append("<sil>")
    else:
        words.append(tok)

def ps_top_words(obj):
    out = []
    for w in obj.get("w", []):
        t = w.get("t", "")
        if t in ("<sil>", "SIL"):
            out.append("<sil>")
        elif not t.isdigit():
            out.append(t)
    return out

ps = json.load(open(ps_json_path, encoding="utf-8"))
ps_words = ps_top_words(ps)

print("sphinx3_align words:", " ".join(words))
print("pocketsphinx words: ", " ".join(ps_words))
print("reference:          ", " ".join(ref_words))

def norm(ws):
    return [w for w in ws if w != "<sil>"]

if norm(words) == norm(ps_words) == ref_words:
    print("ok spike: word sequences match (ignoring inserted silence)")
    sys.exit(0)

mism = []
if norm(words) != norm(ps_words):
    mism.append("sphinx3 vs pocketsphinx")
if norm(ps_words) != ref_words:
    mism.append("pocketsphinx vs reference")
if norm(words) != ref_words:
    mism.append("sphinx3 vs reference")
print("note spike: word mismatch —", ", ".join(mism))
print("(phone/state timing and dictionary mapping may still differ; inspect JSON and outsent)")
sys.exit(0)
PY
