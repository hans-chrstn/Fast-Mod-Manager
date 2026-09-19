#!/usr/bin/env bash
set -Eeuo pipefail

if [[ -n "${PROJECT_SCRIPTS_DIR:-}" ]]; then
  PROJECT_SCRIPT_ROOT="$PROJECT_SCRIPTS_DIR"
else
  PROJECT_SCRIPT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
fi

if [[ -z "${PROJECT_SOURCE_ROOT:-}" ]]; then
  PROJECT_SOURCE_ROOT="$(cd -- "$PROJECT_SCRIPT_ROOT/.." && pwd)"
fi

export PROJECT_SOURCE_ROOT
export PROJECT_SCRIPTS_DIR="$PROJECT_SCRIPT_ROOT"

if [[ -f "$PROJECT_SOURCE_ROOT/lsan.supp" ]]; then
  export LSAN_OPTIONS="suppressions=$PROJECT_SOURCE_ROOT/lsan.supp"
fi

die() {
  printf '%s\n' "$*" >&2
  exit 1
}

require_tool() {
  command -v "$1" >/dev/null 2>&1 || die "required tool not found: $1"
}

require_file() {
  [[ -f "$1" ]] || die "required file not found: $1"
}

require_directory() {
  [[ -d "$1" ]] || die "required directory not found: $1"
}

run_script() {
  local script_name="$1"
  shift
  require_file "$PROJECT_SCRIPTS_DIR/$script_name"
  bash "$PROJECT_SCRIPTS_DIR/$script_name" "$@"
}
