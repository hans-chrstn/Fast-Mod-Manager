#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${PROJECT_SCRIPTS_DIR:-$SCRIPT_DIR}/common.sh"

require_tool cmake
require_tool ninja
require_file "$PROJECT_SOURCE_ROOT/CMakeLists.txt"
require_file "$PROJECT_SOURCE_ROOT/CMakePresets.json"

preset="${1:-debug}"
shift || true

if (( $# > 0 )); then
  die "unexpected arguments: $*"
fi

cd "$PROJECT_SOURCE_ROOT"
cmake --preset "$preset"
