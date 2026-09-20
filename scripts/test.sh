#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${PROJECT_SCRIPTS_DIR:-$SCRIPT_DIR}/common.sh"

require_tool ctest
require_tool cargo
require_file "$PROJECT_SOURCE_ROOT/CMakePresets.json"

preset="${1:-debug}"
shift || true

if (( $# > 0 )); then
  die "unexpected arguments: $*"
fi

cd "$PROJECT_SOURCE_ROOT"
ctest --preset "$preset"

if [[ -d "src/rust" ]]; then
  cd src/rust
  for d in */; do
    if [[ -f "$d/Cargo.toml" ]]; then
      (cd "$d" && cargo test)
    fi
  done
  cd "$PROJECT_SOURCE_ROOT"
fi
