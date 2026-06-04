#!/usr/bin/env bash
# Verify module 000.comp_feat shards: logs under logdir/000.comp_feat and MFC files on disk.
# Use after a failed CI job or before re-running NPART>1 / VTLN training.
#
# Usage:
#   SPIKE_EXPORT_ROOT=/path/to/an4 test/scripts/diagnose_comp_feat.sh
#   LOGDIR=./logdir SPIKE_EXPORT_ROOT=an4 test/scripts/diagnose_comp_feat.sh
#
# CI post-mortem (train-parallel):
#   CHECK_VTLN=no NPART_OVERRIDE=4 SPIKE_EXPORT_ROOT=an4 LOGDIR=an4/logdir ...
#
# CI post-mortem (train-g2p-lda-vtln):
#   VTLN_OVERRIDE=yes NPART_OVERRIDE=2 VTLN_START_OVERRIDE=0.90 VTLN_END_OVERRIDE=1.10 ...
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/../.." && pwd)"
export_root="${SPIKE_EXPORT_ROOT:-}"
if [[ -z "${export_root}" && -d "${repo_root}/../an4/etc" ]]; then
    export_root="${repo_root}/../an4"
fi
if [[ -z "${export_root}" ]]; then
    echo "diagnose_comp_feat: set SPIKE_EXPORT_ROOT to a trained project root" >&2
    exit 2
fi

resolved="${export_root}/etc/sphinx_train.resolved.json"
if [[ ! -f "${resolved}" ]]; then
    echo "diagnose_comp_feat: missing ${resolved}" >&2
    exit 2
fi

check_base="${CHECK_BASE:-yes}"
check_vtln="${CHECK_VTLN:-}"

cfg_file="$(mktemp)"
python3 - "${resolved}" > "${cfg_file}" <<'PY'
import json, sys
v = json.load(open(sys.argv[1], encoding="utf-8"))["variables"]
for k in (
    "CFG_EXPTNAME", "CFG_NPART", "CFG_VTLN", "CFG_VTLN_START", "CFG_VTLN_END",
    "CFG_VTLN_STEP", "CFG_FEATFILES_DIR", "CFG_FEATFILE_EXTENSION", "CFG_LISTOFFILES",
    "DEC_CFG_LISTOFFILES",
):
    print(v.get(k, ""))
PY
expt="$(sed -n '1p' "${cfg_file}")"
nparts="$(sed -n '2p' "${cfg_file}")"
vtln="$(sed -n '3p' "${cfg_file}")"
vtln_start="$(sed -n '4p' "${cfg_file}")"
vtln_end="$(sed -n '5p' "${cfg_file}")"
vtln_step="$(sed -n '6p' "${cfg_file}")"
feat_dir="$(sed -n '7p' "${cfg_file}")"
feat_ext="$(sed -n '8p' "${cfg_file}")"
ctl_train="$(sed -n '9p' "${cfg_file}")"
ctl_test="$(sed -n '10p' "${cfg_file}")"
rm -f "${cfg_file}"

nparts="${NPART_OVERRIDE:-${nparts:-1}}"
vtln="${VTLN_OVERRIDE:-${vtln}}"
vtln_start="${VTLN_START_OVERRIDE:-${vtln_start:-0.80}}"
vtln_end="${VTLN_END_OVERRIDE:-${vtln_end:-1.45}}"
vtln_step="${VTLN_STEP_OVERRIDE:-${vtln_step:-0.05}}"
logdir="${LOGDIR:-${export_root}/logdir}"
comp_logdir="${logdir}/000.comp_feat"
fail=0

if [[ -z "${check_vtln}" ]]; then
    if [[ "${vtln}" == "yes" ]]; then
        check_vtln=yes
    else
        check_vtln=no
    fi
fi

check_shard_logs() {
    local label="$1"
    local logpath="$2"
    if [[ ! -f "${logpath}" ]]; then
        echo "MISSING log (${label}): ${logpath}"
        return 1
    fi
    if ! grep -q 'sphinx_fe.c' "${logpath}" 2>/dev/null; then
        echo "SUSPECT log (${label}, no sphinx_fe output): ${logpath}"
        return 1
    fi
    echo "ok log (${label}): ${logpath}"
    return 0
}

check_shard_mfcs() {
    local label="$1"
    local ctl="$2"
    local out_dir="$3"
    local stats_file
    stats_file="$(mktemp)"
    python3 - "${ctl}" "${out_dir}" "${feat_ext}" "${nparts}" "${label}" <<'PY' > "${stats_file}"
import sys
from pathlib import Path
ctl, out_dir, ext, nparts_s, label = sys.argv[1:6]
nparts = int(nparts_s)
ids = [ln.strip() for ln in Path(ctl).read_text(encoding="utf-8", errors="replace").splitlines() if ln.strip()]
nlines = len(ids)
fail = 0
for part in range(1, nparts + 1):
    offset = (nlines * (part - 1)) // nparts
    count = (nlines * part) // nparts - offset
    slice_ids = ids[offset : offset + count]
    missing = [fid for fid in slice_ids if not (Path(out_dir) / f"{fid}.{ext}").is_file()]
    print(f"{label}\t{part}\t{nparts}\t{count}\t{len(missing)}")
    for fid in missing[:3]:
        print(f"ex\t{fid}")
    if missing:
        fail = 1
sys.exit(fail)
PY
    local py_exit=$?
    while IFS=$'\t' read -r lbl part np cnt miss; do
        [[ "${lbl}" == "ex" || "${lbl}" != "${label}" ]] && continue
        if [[ "${miss}" -eq 0 ]]; then
            echo "ok ${label} part=${part}/${np}: ${cnt} mfcs under ${out_dir}"
        else
            echo "FAIL ${label} part=${part}/${np}: ${miss}/${cnt} mfcs missing under ${out_dir}"
            fail=1
        fi
    done < "${stats_file}"
    if [[ ${py_exit} -ne 0 ]]; then
        grep '^ex' "${stats_file}" | sed -n '1,3p' | while read -r _ ex; do
            echo "  example: ${ex}.${feat_ext}"
        done
    fi
    rm -f "${stats_file}"
}

echo "comp_feat diagnosis: ${export_root}"
echo "  logdir=${comp_logdir} npart=${nparts} check_base=${check_base} check_vtln=${check_vtln}"

if [[ "${check_base}" == "yes" ]]; then
    echo "--- base features (feat/, not warped) ---"
    for part in $(seq 1 "${nparts}"); do
        check_shard_logs "train ${part}/${nparts}" \
            "${comp_logdir}/${expt}.train-${part}-${nparts}.log" || fail=1
    done
    for part in $(seq 1 "${nparts}"); do
        check_shard_logs "test ${part}/${nparts}" \
            "${comp_logdir}/${expt}.test-${part}-${nparts}.log" || fail=1
    done
    check_shard_mfcs "train" "${ctl_train}" "${feat_dir}" || fail=1
    if [[ -n "${ctl_test}" && -f "${ctl_test}" ]]; then
        check_shard_mfcs "test" "${ctl_test}" "${feat_dir}" || fail=1
    fi
fi

if [[ "${check_vtln}" == "yes" ]]; then
    echo "--- VTLN warped features (feat/<warp>/) ---"
    warps="$(python3 - "${vtln_start}" "${vtln_end}" "${vtln_step}" <<'PY'
import sys
start, end, step = (float(x) for x in sys.argv[1:4])
w = start
while w <= end + 1e-9:
    print(f"{w:.2f}")
    w += step
PY
)"
    while IFS= read -r warp; do
        for part in $(seq 1 "${nparts}"); do
            check_shard_logs "train ${part}/${nparts} warp=${warp}" \
                "${comp_logdir}/${expt}.train-${part}-${nparts}-${warp}.log" || fail=1
        done
    done <<< "${warps}"
    while IFS= read -r warp; do
        check_shard_mfcs "train warp=${warp}" "${ctl_train}" "${feat_dir}/${warp}" || fail=1
    done <<< "${warps}"
fi

if [[ ${fail} -eq 0 ]]; then
    echo "ok comp_feat shards look complete"
    exit 0
fi
echo "diagnose_comp_feat: problems found (missing 000.comp_feat logs => parallel job never finished)" >&2
exit 1
