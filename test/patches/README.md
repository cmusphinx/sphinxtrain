# PocketSphinx patches (until upstream merge)

## `pocketsphinx-468-state_align_search.patch`

Upstream: [cmusphinx/pocketsphinx#468](https://github.com/cmusphinx/pocketsphinx/pull/468)

Two-pass / `pocketsphinx align` pruning and phone-transition fixes in
`src/state_align_search.c`. SphinxTrain CI applies this on top of
`POCKETSPHINX_REF` (currently `v5.1.0`) until the PR is merged and the
pin can move to a release tag.

Apply manually:

    test/scripts/apply_pocketsphinx_align_patch.sh /path/to/pocketsphinx

Remove after upstream ships the fix: delete this patch, the apply script,
and the CI apply step; bump `POCKETSPHINX_REF` if needed.
