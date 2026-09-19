#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${PROJECT_SCRIPTS_DIR:-$SCRIPT_DIR}/common.sh"

require_tool rg
require_tool clang-tidy
require_tool cppcheck

preset="${1:-debug}"
shift || true

if (( $# > 0 )); then
  die "unexpected arguments: $*"
fi

compile_commands="$PROJECT_SOURCE_ROOT/build/$preset/compile_commands.json"
require_file "$compile_commands"

cd "$PROJECT_SOURCE_ROOT"
mapfile -t source_files < <(rg --files -g '!build/**' -g '*.cpp' -g '*.cc' -g '*.cxx' | sort)

if (( ${#source_files[@]} == 0 )); then
  exit 0
fi

printf "%s\n" "${source_files[@]}" | xargs -I{} -P "$(nproc)" clang-tidy --quiet -p "build/$preset" "$PROJECT_SOURCE_ROOT/{}" 2>/dev/null
cppcheck --project="$compile_commands" -i "$PROJECT_SOURCE_ROOT/build" --enable=warning,style,performance,portability --error-exitcode=1 --suppress=missingIncludeSystem
