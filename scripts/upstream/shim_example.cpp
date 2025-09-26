// Example shim implementation template for upstream Snes9x
// Copy this file to src/core/upstream/shim_impl.cpp and adapt the code
// to the actual upstream API. This file exposes the small C API the
// adapter expects so the adapter can link against the upstream core.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// The adapter expects these C symbols with the following signatures:
//   extern "C" bool upstream_load_rom(const char* path);
//   extern "C" int upstream_run(unsigned int frames);
//   extern "C" bool upstream_step();
//   extern "C" void upstream_reset();
//   extern "C" bool upstream_save_state(const char* path);
//   extern "C" bool upstream_load_state(const char* path);
//   extern "C" size_t upstream_dump_memory(uint64_t address, uint8_t* out, size_t length);
//   extern "C" const char* upstream_version();

extern "C" {

bool upstream_load_rom(const char* path) {
    // TODO: adapt to upstream API. Example pseudo-code:
    // return S9xLoadROM(path);
    (void)path;
    fprintf(stderr, "shim: upstream_load_rom called with %s\n", path?path:"(null)");
    return true; // change to real result
}

int upstream_run(unsigned int frames) {
    // TODO: run 'frames' frames and return executed frames count or -1 on error
    (void)frames;
    // Example: S9xMainLoop(frames);
    return (int)frames;
}

bool upstream_step() {
    // TODO: step a single frame in upstream
    return true;
}

void upstream_reset() {
    // TODO: reset upstream core
}

bool upstream_save_state(const char* path) {
    // TODO: call upstream's state save function
    (void)path;
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    const char *msg = "shim-state";
    fwrite(msg, 1, strlen(msg), f);
    fclose(f);
    return true;
}

bool upstream_load_state(const char* path) {
    (void)path;
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

size_t upstream_dump_memory(uint64_t address, uint8_t* out, size_t length) {
    if (!out) return 0;
    for (size_t i = 0; i < length; ++i) out[i] = (uint8_t)((address + i) & 0xFF);
    return length;
}

const char* upstream_version() {
    return "upstream-shim-example-0.0";
}

} // extern "C"
