Integrating the upstream Snes9x core

This project provides a clean boundary so you can integrate the real Snes9x core as a submodule or external project.

Recommended steps:

1. Add upstream as a git submodule
   git submodule add <upstream-repo-url> src/core/upstream

2. Ensure the upstream CMake project defines a library target that can be linked. By default this scaffold looks for a target named `snes9x_upstream`.
   - If the upstream CMake target has a different name, set the cache variable when configuring: -DSNES9X_UPSTREAM_TARGET=<upstream-target-name>

3. Implement a thin C-compatible shim in the upstream tree that provides the symbols expected by the adapter (or modify the adapter to call the upstream C++ API directly). The adapter expects these symbols:
   - bool upstream_load_rom(const char* path);
   - int upstream_run(unsigned int frames); // returns number of frames executed or -1 on error
   - bool upstream_step();
   - void upstream_reset();
   - bool upstream_save_state(const char* path);
   - bool upstream_load_state(const char* path);
   - size_t upstream_dump_memory(uint64_t address, uint8_t* out, size_t length);
   - const char* upstream_version();

   The shim should be a tiny wrapper that calls into upstream APIs and exposes predictable C symbols for the adapter to call.

4. Configure the build to use the upstream core:
   cmake -S . -B build -DUSE_UPSTREAM_CORE=ON -DSNES9X_UPSTREAM_TARGET=<target>

5. Build and run tests. The adapter is compiled into the `snes9x_core` target which your upstream target will be linked with.

Notes and licensing
- Do NOT add upstream source files into this repository without preserving their license headers and obeying their terms.
- This project is MIT-licensed; if you include upstream code with a different license, ensure compatibility and attribution as required.
