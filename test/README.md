# SphinxTrain test helpers

## Stage 22 smoke (`run_slt_two_ci_multipron.sh`)

End-to-end check for optional second CI after multipron:

1. Builds a fresh project under `test/work/slt_two_ci_multipron/` (gitignored).
2. Uses `test/fixtures/arctic_slt/etc/` for dictionary, LM, and file lists.
3. Downloads CMU Arctic SLT wav from Festvox on first run (cached under `test/work/corpus_cache/`).
4. Runs `sphinxtrain -s 000,00,20,21,22` with `$CFG_CI_REESTIMATE_AFTER_MULTIPRON = 'yes'`.

Prerequisites: `cmake --build build` (needs `sphinx3_align`).

```bash
SLT_QUICK=1 ./test/run_slt_two_ci_multipron.sh
```

`SLT_QUICK=1` uses the first 32 utterances and two CI iterations. Omit it for the full SLT training list (slow).

Override paths with `SLT_TEMPLATE`, `SLT_WORK`, or `CORPUS_CACHE` if needed.

## Parallel `000.comp_feat` diagnosis

When `NPART>1` with `QUEUE_TYPE::POSIX`, a missing shard shows up later as empty MFCs
(exit 252 from `verify_all.pl`) or a VTLN align log with no `TOTAL FRAMES`.

```bash
# After train-parallel-style setup (NPART=4, no VTLN warps)
CHECK_VTLN=no NPART_OVERRIDE=4 SPIKE_EXPORT_ROOT=/path/to/an4 ./test/scripts/diagnose_comp_feat.sh

# After train-g2p-lda-vtln (NPART=2, VTLN 0.90–1.10)
CHECK_VTLN=yes NPART_OVERRIDE=2 VTLN_START_OVERRIDE=0.90 VTLN_END_OVERRIDE=1.10 \
  SPIKE_EXPORT_ROOT=/path/to/an4 ./test/scripts/diagnose_comp_feat.sh
```

`diagnose_vtln_feats.sh` is a thin wrapper that sets `CHECK_VTLN=yes` only.
CI runs the same checks automatically on failure for `train-parallel` and `train-g2p-lda-vtln`.
