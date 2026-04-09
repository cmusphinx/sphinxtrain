#!/usr/bin/env python
"""
Force-align the training set with sphinx3_align and write SphinxTrain-style transcripts
that reflect the Viterbi-best pronunciation variants (dict word strings from -outsent).

This mirrors scripts/11.force_align/slave_align.pl phases 3–4 (dictionary merge + aligninput)
and scripts/11.force_align/force_align.pl (sphinx3_align invocation), without running the
full SphinxTrain ``sphinxtrain run`` pipeline.

Training layout comes from ``etc/sphinx_train.cfg`` (``CFG_BASE_DIR``, ``CFG_EXPTNAME``,
and the usual align inputs). Outputs go under ``$CFG_BASE_DIR/multipron_align/``.
Normally stage 21 runs this after CI when ``CFG_MULTIPRON`` is not ``no``. Set
``CFG_MULTIPRON`` to ``no`` to disable multipron entirely. Optional:
``CFG_SPHINX3_ALIGN_BINARY`` if ``sphinx3_align`` is not under ``CFG_BIN_DIR``. Beam width
follows ``CFG_FORCE_ALIGN_BEAM``, or ``--beam``, defaulting to ``1e-308`` if unset.

Usage (from project base, after ``sphinxtrain -t TASK setup``)::

  perl scripts/11.force_align/multipron_align.pl [--dry-run] [--first-n N] [--binary PATH]

Or::

  python scripts/11.force_align/multipron_align.py [--dry-run] <project_etc_dir> \\
      [--binary PATH] [--out-dir DIR] [--first-n N] [--beam W]

Environment:
  SPHINX3_ALIGN  overrides the configured sphinx3_align path when set.
"""

from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path

# One-line Perl assignments: $CFG_NAME = value;
_ASSIGN = re.compile(
    r"^\s*\$((?:CFG|ST)[A-Z0-9_]*)\s*=\s*(.*?)\s*;\s*(?:#.*)?$",
    re.MULTILINE,
)


def _parse_sphinx_train_cfg(text: str) -> dict[str, str]:
    raw: dict[str, str] = {}
    for m in _ASSIGN.finditer(text):
        name, val = m.group(1), m.group(2).strip()
        if val.startswith(("'", '"')) and len(val) >= 2 and val[-1] == val[0]:
            raw[name] = val[1:-1]
        else:
            raw[name] = val.rstrip(";").strip()
    out = dict(raw)
    for _ in range(24):
        changed = False
        for k, v in list(out.items()):
            if "$" not in v:
                continue
            nv = v
            for name, repl in out.items():
                nv = nv.replace(f"${{{name}}}", repl)
                nv = nv.replace(f"${name}", repl)
            if nv != v:
                out[k] = nv
                changed = True
        if not changed:
            break
    return out


def _load_cfg(etc: Path) -> dict[str, str]:
    cfg = etc / "sphinx_train.cfg"
    if not cfg.is_file():
        print(f"Missing {cfg}", file=sys.stderr)
        sys.exit(1)
    return _parse_sphinx_train_cfg(cfg.read_text(encoding="utf-8", errors="replace"))


def _build_falign_dicts(
    dictionary: Path,
    filler: Path,
    out_dict: Path,
    out_fdict: Path,
) -> dict[str, str]:
    """Replicate slave_align.pl phase 3: SIL-only fdict; merge other fillers into main dict."""
    silences: dict[str, str] = {}
    fillers: dict[str, str] = {}
    fdict_sil_lines: list[str] = []
    filler_text = filler.read_text(encoding="utf-8", errors="replace").splitlines()
    for line in filler_text:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        word = parts[0]
        phones = parts[1:]
        if len(phones) == 1 and re.search(r"^SIL[be]?$", phones[0], re.I):
            silences[word] = phones[0]
            fdict_sil_lines.append(f"{word}\t{phones[0]}\n")
        else:
            fillers[word] = " ".join(phones)
    out_fdict.write_text("".join(fdict_sil_lines), encoding="utf-8")
    merged = dictionary.read_text(encoding="utf-8", errors="replace")
    for k in sorted(fillers):
        merged += f"{k}\t{fillers[k]}\n"
    out_dict.write_text(merged, encoding="utf-8")
    return silences


def _make_aligninput(
    transcript: Path,
    out_aligninput: Path,
    silences: dict[str, str],
    strip_variant_digits: bool,
) -> None:
    """Replicate slave_align.pl phase 4 (silence removal + optional variant stripping)."""
    lines_out: list[str] = []
    for line in transcript.read_text(encoding="utf-8", errors="replace").splitlines():
        s = line.rstrip("\n")
        for sil_word in silences:
            s = re.sub(rf"(^|\s){re.escape(sil_word)}(\s|$)", r"\1\2", s)
        if strip_variant_digits:
            s = re.sub(r"\(\d+\)", "", s)
        s = s.strip()
        s = re.sub(r"\s+", " ", s)
        lines_out.append(s + "\n")
    out_aligninput.write_text("".join(lines_out), encoding="utf-8")


def _outsent_to_transcription(outsent: Path, out_transcription: Path) -> None:
    """sphinx3_align -outsent -> SphinxTrain ``<s> ... </s> (uttid)`` (single wrap)."""
    lines: list[str] = []
    for line in outsent.read_text(encoding="utf-8", errors="replace").splitlines():
        line = line.strip()
        if not line:
            continue
        m = re.search(r"\(([^()]+)\)\s*$", line)
        if not m:
            print(f"Skipping unparsable outsent line: {line!r}", file=sys.stderr)
            continue
        uttid = m.group(1).strip()
        words = line[: m.start()].strip().split()
        while words and words[0] == "<s>":
            words = words[1:]
        while words and words[-1] == "</s>":
            words = words[:-1]
        inner = " ".join(words)
        if not inner:
            lines.append(f"<s> </s> ({uttid})\n")
            continue
        lines.append(f"<s> {inner} </s> ({uttid})\n")
    out_transcription.write_text("".join(lines), encoding="utf-8")


def _write_skipped_utts_from_log(log_file: Path, out_path: Path) -> None:
    if not log_file.is_file():
        return
    text = log_file.read_text(encoding="utf-8", errors="replace")
    ids: list[str] = []
    for line in text.splitlines():
        if "Wrote -outsent fallback" not in line:
            continue
        m = re.search(r"for (\S+)\s*$", line)
        if m:
            ids.append(m.group(1))
    if not ids:
        return
    out_path.write_text("".join(f"{u}\n" for u in ids), encoding="utf-8")
    print(f"Wrote skipped-utterance list ({len(ids)}): {out_path}")


def main() -> int:
    argv = sys.argv[1:]
    dry_run = False
    if argv and argv[0] == "--dry-run":
        dry_run = True
        argv = argv[1:]
    if not argv:
        print(__doc__, file=sys.stderr)
        return 2
    etc = Path(argv[0]).resolve()
    bin_override: str | None = None
    out_dir: Path | None = None
    first_n: int | None = None
    beam_override: str | None = None
    rest = argv[1:]
    i = 0
    while i < len(rest):
        if rest[i] == "--binary" and i + 1 < len(rest):
            bin_override = rest[i + 1]
            i += 2
            continue
        if rest[i] == "--out-dir" and i + 1 < len(rest):
            out_dir = Path(rest[i + 1]).resolve()
            i += 2
            continue
        if rest[i] == "--first-n" and i + 1 < len(rest):
            first_n = int(rest[i + 1])
            i += 2
            continue
        if rest[i] == "--beam" and i + 1 < len(rest):
            beam_override = rest[i + 1]
            i += 2
            continue
        print(f"Unexpected argument: {rest[i]}", file=sys.stderr)
        return 2
    cfg = _load_cfg(etc)
    base_dir = Path(cfg["CFG_BASE_DIR"])
    expt = cfg["CFG_EXPTNAME"]
    out_root = out_dir if out_dir is not None else base_dir / "multipron_align"

    align_bin = (
        bin_override
        or os.environ.get("SPHINX3_ALIGN")
        or cfg.get("CFG_SPHINX3_ALIGN_BINARY")
        or str(Path(cfg["CFG_BIN_DIR"]) / "sphinx3_align")
    )
    align_path = Path(align_bin)
    if not dry_run and not align_path.is_file():
        print(
            f"sphinx3_align not found at {align_path} "
            "(build the sphinx3_align target, set CFG_SPHINX3_ALIGN_BINARY in the project "
            "cfg, or set SPHINX3_ALIGN / --binary).",
            file=sys.stderr,
        )
        return 1

    dictionary = Path(cfg["CFG_DICTIONARY"])
    filler = Path(cfg["CFG_FILLERDICT"])
    listoffiles = Path(cfg["CFG_LISTOFFILES"])
    transcript = Path(cfg["CFG_TRANSCRIPTFILE"])
    ctlcount = "1000000"
    feat_dir = Path(cfg["CFG_FEATFILES_DIR"])
    feat_ext = "." + cfg["CFG_FEATFILE_EXTENSION"].lstrip(".")
    hmm_dir = Path(cfg["CFG_MODEL_DIR"]) / f"{expt}.ci_{cfg['CFG_DIRLABEL']}"

    if not hmm_dir.is_dir():
        print(f"Missing HMM directory {hmm_dir} (train CI models first).", file=sys.stderr)
        return 1
    if not feat_dir.is_dir():
        print(f"Missing features directory {feat_dir} (run feature extraction).", file=sys.stderr)
        return 1
    if not transcript.is_file():
        print(f"Missing transcript {transcript}", file=sys.stderr)
        return 1

    out_transcription = out_root / f"{expt}.multipron.transcription"

    falign_dict = out_root / f"{expt}.falign.dict"
    falign_fdict = out_root / f"{expt}.falign.fdict"
    aligninput = out_root / f"{expt}.aligninput"
    ctl_path = listoffiles
    if first_n is not None and first_n > 0:
        ctl_path = out_root / f"{expt}.multipron.shorter.fileids"
        ctlcount = str(first_n)
    outsent = out_root / f"{expt}.multipron.outsent"
    log_file = out_root / f"{expt}.multipron_align.log"

    # sphinx3_align: -beam is a linear probability passed to logs3(); smaller p =>
    # wider Viterbi pruning (see s3_align.c). Default 1e-308 is effectively full width.
    beam = beam_override or cfg.get("CFG_FORCE_ALIGN_BEAM") or "1e-308"
    statepdeffn = cfg["CFG_HMM_TYPE"]
    mwfloor = "1e-8"
    minvar = "1e-4"

    args = [
        str(align_path),
        "-hmm",
        str(hmm_dir),
        "-senmgau",
        statepdeffn,
        "-mixwfloor",
        mwfloor,
        "-varfloor",
        minvar,
        "-dict",
        str(falign_dict),
        "-fdict",
        str(falign_fdict),
        "-ctl",
        str(ctl_path),
        "-ctloffset",
        "0",
        "-ctlcount",
        ctlcount,
        "-cepdir",
        str(feat_dir),
        "-cepext",
        feat_ext,
        "-insent",
        str(aligninput),
        "-outsent",
        str(outsent),
        "-beam",
        beam,
        "-agc",
        cfg["CFG_AGC"],
        "-cmn",
        cfg["CFG_CMN"],
        "-varnorm",
        cfg["CFG_VARNORM"],
        "-feat",
        cfg["CFG_FEATURE"],
        "-ceplen",
        cfg["CFG_VECTOR_LENGTH"],
        "-insert_sil",
        "1",
    ]

    print("Doing multipron force alignment (sphinx3_align)...")
    if dry_run:
        print("Dry run: would create dicts and aligninput under:\n ", out_root)
        print("Dry run: would run:\n ", " ".join(args))
        print(f"Would write transcripts to {out_transcription}")
        return 0

    out_root.mkdir(parents=True, exist_ok=True)
    print(f"Writing merged dicts under {out_root}...")
    silences = _build_falign_dicts(dictionary, filler, falign_dict, falign_fdict)
    _make_aligninput(transcript, aligninput, silences, strip_variant_digits=True)
    if first_n is not None and first_n > 0:
        ids = listoffiles.read_text(encoding="utf-8", errors="replace").splitlines()
        ins_lines = aligninput.read_text(encoding="utf-8", errors="replace").splitlines()
        n = min(first_n, len(ids), len(ins_lines))
        ctl_path.write_text("\n".join(ids[:n]) + "\n", encoding="utf-8")
        aligninput.write_text("\n".join(ins_lines[:n]) + "\n", encoding="utf-8")
        print(f"Limited run to first {n} utterances (--first-n).")

    log_file.write_text(
        "Command:\n" + " ".join(args) + "\n\n", encoding="utf-8"
    )
    with log_file.open("ab") as logf:
        proc = subprocess.run(args, stdout=logf, stderr=subprocess.STDOUT)
    if proc.returncode != 0:
        print(f"sphinx3_align failed (see {log_file})", file=sys.stderr)
        return proc.returncode
    if not outsent.is_file():
        print(f"Expected output missing: {outsent}", file=sys.stderr)
        return 1
    _outsent_to_transcription(outsent, out_transcription)
    skipped_utts = out_root / f"{expt}.multipron.skipped_utts"
    _write_skipped_utts_from_log(log_file, skipped_utts)
    print(f"Wrote {outsent}")
    print(f"Wrote {out_transcription}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
