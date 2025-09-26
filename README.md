snes9x-ci: Headless Snes9x fork for CI

Goal:
- Provide a headless, CLI-first wrapper and clean core boundary so Snes9x can be used in CI pipelines for smoke testing, regression automation, and deterministic runs.

This repository is a scaffold and stub implementation; it intentionally does NOT include upstream Snes9x source code.

Getting started (local build):

1. mkdir build && cd build
2. cmake ..
3. cmake --build . --config Release
4. ./snes9x-cli --help

Adding the upstream Snes9x core:
- Add the upstream sources as a git submodule or copy sources into src/core. The public headers and API should be implemented in include/core_api.h and the library target `snes9x_core` should be mapped to the actual core sources.

Integration with upstream core
- Use the CMake option `-DUSE_UPSTREAM_CORE=ON` to enable building an adapter that links against an upstream library.
- By default the adapter expects an upstream CMake target named `snes9x_upstream`. You can override this by passing `-DSNES9X_UPSTREAM_TARGET=<target-name>` to CMake.
- See `docs/INTEGRATING.md` for detailed integration steps and guidance for writing a small shim to expose C symbols to the adapter.

CI:
- A GitHub Actions workflow is provided at .github/workflows/ci.yml to build across platforms, run tests, and execute a headless smoke test.

Usage examples

Basic run:
  ./snes9x-cli --load path/to/rom.sfc --run 1 --json-output

Dump memory (human-readable):
  ./snes9x-cli --load path/to/rom.sfc --dump-memory 0x100 16

Save/load state:
  ./snes9x-cli --load path/to/rom.sfc --save-state /tmp/state.bin
  ./snes9x-cli --load-state /tmp/state.bin --run 1 --json-output

Run script (deterministic sequences):
  ./snes9x-cli --run-script tests/data/script.txt --json-output

Example script file (tests/data/script.txt):
  # comment lines start with '#'
  load dummy.sfc
  run 1
  dump-memory 0x100 8
  save-state smoke-state.bin

JSON output
-------------
The CLI exposes a minimal machine-friendly JSON output format when `--json-output` is used. Example:

  {"success":true,"frames":1,"message":"ok"}

The fields are:
  - success: boolean indicating overall success
  - frames: number of frames executed (for run)
  - message: short human-friendly status message

Exit codes
-------------
See `include/exit_codes.h` for numeric exit codes. Typical codes used by CI:
  - 0: success
  - 3: EXIT_LOAD_ROM_FAILED
  - 4: EXIT_LOAD_STATE_FAILED
  - 5: EXIT_RUN_FAILED
  - 6: EXIT_STEP_FAILED
  - 7: EXIT_SAVE_STATE_FAILED

CI example (assert JSON pass)
--------------------------------
This repository includes a small Python helper `tools/ci_smoke_test.py` which runs the CLI and asserts the JSON result. Example GitHub Actions step:

  - name: Run headless CLI smoke test
    run: python3 tools/ci_smoke_test.py build/snes9x-cli build/tests/data/dummy.sfc

Use this helper in CI to get structured failure codes and clearer debug logs when the CLI reports errors.

Further docs
-------------
See `docs/INTEGRATING.md` for upstream integration notes and `docs/USAGE.md` for a more detailed command reference and script examples.

License and contributing:
- This scaffold is MIT-licensed. When adding upstream Snes9x code, carefully preserve upstream licensing and attribution in files and in this repository.
