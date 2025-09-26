#pragma once

namespace snes9x {

enum ExitCode {
    EXIT_SUCCESSFUL = 0,
    EXIT_USAGE = 1,
    EXIT_CLI_NOT_FOUND = 2,
    EXIT_LOAD_ROM_FAILED = 3,
    EXIT_LOAD_STATE_FAILED = 4,
    EXIT_RUN_FAILED = 5,
    EXIT_STEP_FAILED = 6,
    EXIT_SAVE_STATE_FAILED = 7,
    EXIT_DUMP_MEMORY_FAILED = 8,
    EXIT_SCRIPT_FAILED = 9,
    EXIT_INTERNAL_ERROR = 10
};

} // namespace snes9x
