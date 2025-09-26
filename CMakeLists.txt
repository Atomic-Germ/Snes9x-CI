cmake_minimum_required(VERSION 3.15)
project(snes9x-ci VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_SHARED_CORE "Build core as shared library" OFF)
option(ENABLE_SANITIZERS "Enable ASAN/UBSAN" OFF)
option(HEADLESS_ONLY "Build headless-only without GUI" ON)
option(USE_UPSTREAM_CORE "Use an upstream Snes9x core submodule or external project" OFF)
set(SNES9X_UPSTREAM_TARGET "" CACHE STRING "If using an upstream core, set the CMake target name to link against (optional)")

if(ENABLE_SANITIZERS)
  message(STATUS "Enabling sanitizers: Address and Undefined Behavior")
  add_compile_options(-fsanitize=address -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

# Export a LIB_SUFFIX for CI artifacts
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
endif()

message(STATUS "USE_UPSTREAM_CORE=${USE_UPSTREAM_CORE}")
message(STATUS "SNES9X_UPSTREAM_TARGET=${SNES9X_UPSTREAM_TARGET}")

add_subdirectory(src/core)
add_subdirectory(src/cli)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
