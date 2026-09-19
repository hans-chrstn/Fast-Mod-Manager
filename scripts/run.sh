#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${PROJECT_SCRIPTS_DIR:-$SCRIPT_DIR}/common.sh"

require_tool cmake
require_file "$PROJECT_SOURCE_ROOT/CMakePresets.json"

preset="${1:-debug}"
if [[ $# -gt 0 ]]; then
  shift
fi

run_script build.sh "$preset"
executable="$PROJECT_SOURCE_ROOT/build/$preset/mod-manager"
require_file "$executable"
exec "$executable" "$@"
