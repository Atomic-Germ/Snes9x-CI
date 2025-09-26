#pragma once

#include <string>
#include <vector>

namespace snes9x {

struct RunResult {
    bool success;
    int frames_executed;
    std::string message; // machine-friendly short message
};

class Core {
public:
    Core();
    ~Core();

    // Load a ROM from path. Returns true on success.
    bool loadRom(const std::string &path);

    // Run until stopped or for 'frames' frames if frames > 0.
    RunResult run(unsigned int frames = 0);

    // Step a single emulation step (frame). Returns true on success.
    bool step();

    // Reset the core to power-on state.
    void reset();

    // Save and load state to a file.
    bool saveState(const std::string &path) const;
    bool loadState(const std::string &path);

    // Dump a block of memory (address, length). For stub returns a vector of bytes.
    std::vector<uint8_t> dumpMemory(uint64_t address, size_t length) const;

    // Version string for the core
    static std::string version();
};

} // namespace snes9x
