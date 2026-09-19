#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${PROJECT_SCRIPTS_DIR:-$SCRIPT_DIR}/common.sh"

require_tool shellcheck
require_file "$PROJECT_SOURCE_ROOT/flake.lock"

preset="${1:-debug}"
shift || true

if (( $# > 0 )); then
  die "unexpected arguments: $*"
fi

cd "$PROJECT_SOURCE_ROOT"
shellcheck -x -S warning "$PROJECT_SCRIPTS_DIR"/*.sh
run_script format.sh --check
run_script configure.sh "$preset"
run_script build.sh "$preset"
run_script test.sh "$preset"
run_script lint.sh "$preset"
