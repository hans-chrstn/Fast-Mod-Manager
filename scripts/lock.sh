#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${PROJECT_SCRIPTS_DIR:-$SCRIPT_DIR}/common.sh"

require_tool nix
require_file "$PROJECT_SOURCE_ROOT/flake.nix"

cd "$PROJECT_SOURCE_ROOT"
nix flake lock
