#include "core_api.h"
#include <stdexcept>
#include <string>

using namespace snes9x;

// Adapter glue: When integrating the upstream core, provide C linkage entry points or
// an upstream C++ target that exposes these symbols. The upstream project should
// either provide a target named `snes9x_upstream` or set the CMake cache variable
// `SNES9X_UPSTREAM_TARGET` to point to its library target. The adapter expects the
// following C-style symbols to be available at link time (implementations should
// be provided by the upstream or by a small wrapper):
//   extern "C" bool upstream_load_rom(const char* path);
//   extern "C" int upstream_run(unsigned int frames);
//   extern "C" bool upstream_step();
//   extern "C" void upstream_reset();
//   extern "C" bool upstream_save_state(const char* path);
//   extern "C" bool upstream_load_state(const char* path);
//   extern "C" size_t upstream_dump_memory(uint64_t address, uint8_t* out, size_t length);
//   extern "C" const char* upstream_version();

extern "C" {
    // Declarations only; upstream must provide definitions
    bool upstream_load_rom(const char* path);
    int upstream_run(unsigned int frames);
    bool upstream_step();
    void upstream_reset();
    bool upstream_save_state(const char* path);
    bool upstream_load_state(const char* path);
    size_t upstream_dump_memory(uint64_t address, uint8_t* out, size_t length);
    const char* upstream_version();
}

Core::Core() {}
Core::~Core() {}

bool Core::loadRom(const std::string &path) {
    return upstream_load_rom(path.c_str());
}

RunResult Core::run(unsigned int frames) {
    RunResult r{true, 0, "ok"};
    int executed = upstream_run(frames);
    if (executed < 0) {
        r.success = false;
        r.message = "upstream run failed";
    } else {
        r.frames_executed = executed;
    }
    return r;
}

bool Core::step() {
    return upstream_step();
}

void Core::reset() {
    upstream_reset();
}

bool Core::saveState(const std::string &path) const {
    return upstream_save_state(path.c_str());
}

bool Core::loadState(const std::string &path) {
    return upstream_load_state(path.c_str());
}

std::vector<uint8_t> Core::dumpMemory(uint64_t address, size_t length) const {
    std::vector<uint8_t> buf(length);
    size_t got = upstream_dump_memory(address, buf.data(), length);
    buf.resize(got);
    return buf;
}

std::string Core::version() {
    const char *v = upstream_version();
    return v ? std::string(v) : std::string("unknown-upstream");
}
