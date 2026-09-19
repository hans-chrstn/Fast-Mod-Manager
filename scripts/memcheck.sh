#!/usr/bin/env bash
set -euo pipefail

build_type=${1:-debug}
build_dir="$PROJECT_SOURCE_ROOT/build/$build_type"

if [ ! -d "$build_dir" ]; then
    echo "Build directory $build_dir does not exist. Run configure and build first."
    exit 1
fi

echo "Running unit tests through valgrind..."
cd "$build_dir"
valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=definite --error-exitcode=1 ./tests/mod-manager-tests

echo "Valgrind memcheck completed successfully."
