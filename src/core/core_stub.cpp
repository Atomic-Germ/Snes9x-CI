#include "core_api.h"
#include <thread>
#include <chrono>

using namespace snes9x;

Core::Core() {}
Core::~Core() {}

bool Core::loadRom(const std::string &path) {
    // Stub: accept any non-empty path and "load" it
    return !path.empty();
}

RunResult Core::run(unsigned int frames) {
    RunResult r{true, 0, "ok"};
    if (frames == 0) {
        // run for a short simulated period then exit
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        r.frames_executed = 1;
        return r;
    }

    for (unsigned int i = 0; i < frames; ++i) {
        // simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        r.frames_executed++;
    }
    return r;
}

bool Core::step() {
    // simulate a single step
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return true;
}

void Core::reset() {
    // no-op in stub
}

bool Core::saveState(const std::string &path) const {
    // create a tiny file to simulate state save
    FILE *f = fopen(path.c_str(), "wb");
    if (!f) return false;
    const char *msg = "stub-state";
    fwrite(msg, 1, strlen(msg), f);
    fclose(f);
    return true;
}

bool Core::loadState(const std::string &path) {
    // succeed if file exists
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

std::vector<uint8_t> Core::dumpMemory(uint64_t address, size_t length) const {
    std::vector<uint8_t> buf(length);
    for (size_t i = 0; i < length; ++i) buf[i] = static_cast<uint8_t>((address + i) & 0xFF);
    return buf;
}

std::string Core::version() {
    return "snes9x-ci-core-stub-0.1";
}
