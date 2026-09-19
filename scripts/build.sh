#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${PROJECT_SCRIPTS_DIR:-$SCRIPT_DIR}/common.sh"

require_tool cmake
require_file "$PROJECT_SOURCE_ROOT/CMakePresets.json"

preset="${1:-debug}"
shift || true

if (( $# > 0 )); then
  die "unexpected arguments: $*"
fi

cd "$PROJECT_SOURCE_ROOT"

if [[ -n "${BUILD_JOBS:-}" ]]; then
  cmake --build --preset "$preset" --parallel "$BUILD_JOBS"
else
  cmake --build --preset "$preset" --parallel
fi
