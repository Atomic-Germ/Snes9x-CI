cmake_minimum_required(VERSION 3.15)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.14.0.zip
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(core_smoke_test core_smoke_test.cpp)

target_link_libraries(core_smoke_test PRIVATE GTest::gtest GTest::gtest_main snes9x_core)

enable_testing()
add_test(NAME core_smoke_test COMMAND core_smoke_test)

# Copy test data into the build directory so CLI can load it
file(COPY ${CMAKE_SOURCE_DIR}/tests/data DESTINATION ${CMAKE_BINARY_DIR}/tests)

# CLI smoke test: run the built CLI against the dummy ROM and request JSON output
add_test(NAME cli_smoke_test
  COMMAND $<TARGET_FILE:snes9x-cli> --load ${CMAKE_BINARY_DIR}/tests/data/dummy.sfc --run 1 --json-output
)

# CLI: save state test
add_test(NAME cli_save_state_test
  COMMAND $<TARGET_FILE:snes9x-cli> --load ${CMAKE_BINARY_DIR}/tests/data/dummy.sfc --save-state ${CMAKE_BINARY_DIR}/tests/data/test-state.bin --json-output
)

# CLI: dump memory test
add_test(NAME cli_dump_memory_test
  COMMAND $<TARGET_FILE:snes9x-cli> --load ${CMAKE_BINARY_DIR}/tests/data/dummy.sfc --dump-memory 0x100 8 --json-output
)

# CLI: run-script test
add_test(NAME cli_script_test
  COMMAND $<TARGET_FILE:snes9x-cli> --run-script ${CMAKE_BINARY_DIR}/tests/data/script.txt --json-output
)

# CLI: script that writes pretty output to a file
add_test(NAME cli_script_pretty_test
  COMMAND $<TARGET_FILE:snes9x-cli> --run-script ${CMAKE_BINARY_DIR}/tests/data/script_pretty.txt --format pretty --output-file ${CMAKE_BINARY_DIR}/tests/data/pretty-output.txt
)

# CLI: script that writes jsonl output to a file
add_test(NAME cli_script_outputfile_test
  COMMAND $<TARGET_FILE:snes9x-cli> --run-script ${CMAKE_BINARY_DIR}/tests/data/script_outputfile.txt --format jsonl --output-file ${CMAKE_BINARY_DIR}/tests/data/smoke_output.jsonl
)

# Verify the assert script via helper
add_test(NAME cli_script_assert_test
  COMMAND $<TARGET_FILE:snes9x-cli> --run-script ${CMAKE_BINARY_DIR}/tests/data/script_assert.txt --format jsonl --output-file ${CMAKE_BINARY_DIR}/tests/data/assert_output.jsonl
)

# Use the helper to check the output file contains expected JSON
add_test(NAME check_assert_output
  COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/check_output_file.py ${CMAKE_BINARY_DIR}/tests/data/assert_output.jsonl success true
)
