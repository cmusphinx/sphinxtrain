#!/usr/bin/env bash
# Wrapper: VTLN warped-feature checks only (see diagnose_comp_feat.sh).
set -euo pipefail
root="$(cd "$(dirname "$0")" && pwd)"
export CHECK_BASE=no
export CHECK_VTLN=yes
exec "${root}/diagnose_comp_feat.sh" "$@"
