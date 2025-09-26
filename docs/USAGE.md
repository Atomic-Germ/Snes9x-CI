Command-line usage and examples

This document expands on the CLI usage and provides examples for CI-oriented workflows.

Commands

- --load <path>
  Load a ROM file. The CLI returns a failure exit code if loading fails.

- --run [frames]
  Run the emulator. If frames is omitted or 0, the CLI performs a brief run useful for smoke tests.

- --step
  Execute a single emulation step/frame.

- --reset
  Reset the core to the power-on state.

- --save-state <path>
  Write a savestate to disk for later inspection. Useful in CI when capturing failing state.

- --load-state <path>
  Load a previously saved state and resume.

- --dump-memory <addr> <len>
  Return a block of memory. By default the CLI prints hex; use --json-output for machine parsing.

- --run-script <path>
  Execute a sequence of commands from a script file. The script language is intentionally tiny and supports commands like: load, run, step, save-state, load-state, dump-memory, reset, sleep and allows comments starting with '#'.

Formatting and output

- --format <json|jsonl|pretty>
  Choose the output format. `json` prints a single JSON document to stdout, `jsonl` (NDJSON) appends JSON lines to the selected output file or stdout, and `pretty` prints a multi-line human readable report.

- --output-file <path>
  Append output to the specified file instead of printing to stdout. When used with `jsonl` the file will contain one JSON document per emitted event which is convenient for CI log aggregation.

JSON output

When --json-output is specified, the CLI produces a single JSON document for commands that return a structured result (run, step, run-script, dump-memory). Rely on the `success` field to gate CI steps.

Script language extensions

- set <VAR> <value>
  Define a variable usable later in the script.

- Variable substitution: use ${VAR} to substitute previously set variables in command arguments.

- expect-json <key> <value>
  Assert that the last emitted JSON (from file when using --output-file) contains the given key with a matching value. The script will fail if the assertion doesn't hold; useful for CI checks embedded in scripts.

Script example

  # tests/data/script.txt
  load dummy.sfc
  run 1
  dump-memory 0x100 8
  save-state smoke-state.bin

CI integration patterns

- Use `tools/ci_smoke_test.py` to run the CLI and assert JSON output. The script prints helpful STDOUT and returns non-zero on failure.
- Capture saves and logs as artifacts when a CI job fails to enable local reproduction.

Exit codes

See `include/exit_codes.h` for the full list. CI should rely on numeric exit codes or the JSON `success` field for machine assertions.

Timestamped logging

- --timestamp
  When provided, emitted JSON events will include a `timestamp` field with an ISO8601 UTC timestamp at the time of emission. For JSONL/NDJSON outputs this provides easy sorting and ordering in CI log collectors.

JSON schema validation

- Use `tools/json_schema_validate.py <file>` to validate a single JSON document or a JSONL file produced by the CLI against the expected output shape. This tool is lightweight and has no external dependencies.

Examples

Write NDJSON to a file and assert success:

  set OUTFILE build/tests/data/assert_output.jsonl
  load dummy.sfc
  run 1
  expect-json success true

This will append a JSON line to `OUTFILE` and then verify the last line contains success=true.

Example: validate a produced JSONL

  ./snes9x-cli --run-script tests/data/script_outputfile.txt --format jsonl --output-file build/tests/data/smoke_output.jsonl --timestamp
  python3 tools/json_schema_validate.py build/tests/data/smoke_output.jsonl
