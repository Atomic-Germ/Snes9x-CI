Contributing

- This repo is a clean-room scaffold for a headless Snes9x CI tool. Do NOT add upstream Snes9x source files directly into the repository without preserving original license headers and attribution.
- Preferred method to integrate upstream: add as a git submodule under src/core/upstream or provide a documented step to copy sources into src/core.
- Maintainers should ensure license compatibility before merging upstream code.
- See README.md for build instructions.

Upstream integration checklist
- Add the upstream repo as a git submodule at `src/core/upstream`.
- Ensure the upstream project defines a CMake target for its core library (default expected target name: `snes9x_upstream`).
- Provide a thin C-compatible shim or wrapper in the upstream project that exposes the C symbols documented in `docs/INTEGRATING.md`. The shim allows this adapter to call upstream code without embedding upstream internals here.
- When submitting a PR that integrates upstream code, include a brief note describing how licensing and attribution have been preserved.

CI guidance
- Use the `tools/ci_smoke_test.py` helper to run the CLI and assert JSON output. It prints useful STDOUT and STDERR on failures to help debugging.
- Configure your GitHub Actions workflow to upload the `build` directory on failure; this repo's `ci.yml` already does that.
- When a CI job fails, download the artifact named `build-logs` from the workflow run to reproduce locally.
