#!/usr/bin/env bash
set -euo pipefail

# Regression runner for snes9x-ci
# - Ensures project is configured and built
# - Runs all scripts in tests/data/regression and writes NDJSON outputs to build/regression
# - Exits non-zero if any script fails

ROOT_DIR=$(pwd)
BUILD_DIR=${BUILD_DIR:-build}
CLI=${CLI:-${BUILD_DIR}/src/cli/snes9x-cli}
REG_DIR=${REG_DIR:-${BUILD_DIR}/regression}
SCRIPTS_DIR=${SCRIPTS_DIR:-tests/data/regression}

mkdir -p "$BUILD_DIR"
if [ ! -x "$CLI" ]; then
  echo "CLI not found at $CLI; running a build"
  cmake -S . -B "$BUILD_DIR" -DBUILD_TESTS=ON
  cmake --build "$BUILD_DIR" -- -j 2
fi

mkdir -p "$REG_DIR"

failures=0
for script in "$SCRIPTS_DIR"/*.txt; do
  [ -e "$script" ] || continue
  name=$(basename "$script" .txt)
  out="$REG_DIR/${name}.jsonl"
  echo "Running regression script: $script -> $out"
  # remove previous output
  rm -f "$out"
  # run CLI; format jsonl and include timestamp
  if ! "$CLI" --run-script "$script" --format jsonl --output-file "$out" --timestamp; then
    echo "Script failed: $script"
    failures=$((failures+1))
  fi
done

if [ $failures -ne 0 ]; then
  echo "$failures regression script(s) failed"
  exit 2
fi

echo "All regression scripts passed"
exit 0
