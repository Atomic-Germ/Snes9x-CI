#!/usr/bin/env bash
set -euo pipefail

# Example script to run a ROM headless in CI
BUILD_DIR=${BUILD_DIR:-build}
CLI=${CLI:-${BUILD_DIR}/snes9x-cli}
ROM=${1:-dummy.sfc}

if [ ! -x "$CLI" ]; then
  echo "CLI binary not found at $CLI. Build the project first (mkdir build && cd build && cmake .. && cmake --build .)"
  exit 2
fi

# Run a short headless run and emit JSON
"$CLI" --load "$ROM" --run 1 --json-output
