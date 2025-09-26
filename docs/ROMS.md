Included test ROMs

This repository includes two open-source test ROM images intended for emulator testing and verification:

- tools/240p_Test_Suite_NTSC.sfc — The NTSC variant of the 240p Test Suite.
- tools/240p_Test_Suite_PAL.sfc — The PAL variant.

These ROMs are used as deterministic test fixtures and smoke/regression inputs. The upstream source is the 240p Test Suite project; the sources are available for inspection to understand expected behavior (e.g. timings and specific test patterns) but the ROM images are included here for convenience.

When adding or updating ROM files:
- Preserve and include any upstream license or attribution.
- Consider adding test scripts under `tests/data/regression` that exercise the ROM and emit machine-readable outputs (JSON/NDJSON) for automated validation.

Example usage (local):
  ./snes9x-cli --load tools/240p_Test_Suite_NTSC.sfc --run 60 --json-output

Note: The included ROMs are small test fixtures and are not the full emulator; they are intended for testing the emulator core and CI workflows.
