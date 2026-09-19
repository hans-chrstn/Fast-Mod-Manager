#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${PROJECT_SCRIPTS_DIR:-$SCRIPT_DIR}/common.sh"

require_tool rg
require_tool clang-format

mode="write"
if [[ "${1:-}" == "--check" ]]; then
  mode="check"
  shift
fi

if (( $# > 0 )); then
  die "unexpected arguments: $*"
fi

cd "$PROJECT_SOURCE_ROOT"
mapfile -t source_files < <(rg --files -g '!build/**' -g '*.c' -g '*.cc' -g '*.cpp' -g '*.cxx' -g '*.h' -g '*.hh' -g '*.hpp' -g '*.hxx' | sort)

if (( ${#source_files[@]} == 0 )); then
  exit 0
fi

if [[ "$mode" == "check" ]]; then
  clang-format --dry-run --Werror "${source_files[@]}"
else
  clang-format -i "${source_files[@]}"
fi
