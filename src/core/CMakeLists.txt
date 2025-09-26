cmake_minimum_required(VERSION 3.15)

if(USE_UPSTREAM_CORE)
  # If upstream sources exist under src/core/upstream, add them so they can define their own targets
  if(EXISTS ${CMAKE_SOURCE_DIR}/src/core/upstream/CMakeLists.txt)
    add_subdirectory(upstream)
  else()
    message(STATUS "USE_UPSTREAM_CORE is ON but no upstream CMakeLists.txt found under src/core/upstream. You must add the upstream core as a submodule or provide a CMake project at that path.")
  endif()

  # If upstream didn't provide an explicit CMake target, try to locate a built library
  if(NOT TARGET snes9x_upstream)
    # Common locations to search for built upstream artifacts
    file(GLOB_RECURSE _upstream_libs
      "${CMAKE_SOURCE_DIR}/src/core/upstream/build/*/*.a"
      "${CMAKE_SOURCE_DIR}/src/core/upstream/build/*.a"
      "${CMAKE_SOURCE_DIR}/src/core/upstream/*/lib*.a"
      "${CMAKE_SOURCE_DIR}/src/core/upstream/*/lib*.dylib"
      "${CMAKE_SOURCE_DIR}/src/core/upstream/*/lib*.so"
    )
    list(LENGTH _upstream_libs _count)
    if(_count GREATER 0)
      list(GET _upstream_libs 0 _first_upstream_lib)
      message(STATUS "Found upstream library: ${_first_upstream_lib}; creating IMPORTED target 'snes9x_upstream'")
      add_library(snes9x_upstream STATIC IMPORTED)
      set_target_properties(snes9x_upstream PROPERTIES IMPORTED_LOCATION "${_first_upstream_lib}")
    else()
      message(STATUS "No upstream library found automatically; you can run scripts/upstream/fetch_and_build_snes9x.sh to build upstream or set SNES9X_UPSTREAM_TARGET to a target provided by upstream.")
    endif()
  endif()

  # Build an adapter that maps the upstream core into our snes9x_core target
  if(BUILD_SHARED_CORE)
    add_library(snes9x_core SHARED core_adapter.cpp)
  else()
    add_library(snes9x_core STATIC core_adapter.cpp)
  endif()

  target_include_directories(snes9x_core
    PUBLIC
      ${CMAKE_SOURCE_DIR}/include
  )

  # Link the adapter to the upstream target. Allow the user to override the upstream target name.
  if(SNES9X_UPSTREAM_TARGET)
    if(TARGET ${SNES9X_UPSTREAM_TARGET})
      target_link_libraries(snes9x_core PRIVATE ${SNES9X_UPSTREAM_TARGET})
    else()
      message(FATAL_ERROR "SNES9X_UPSTREAM_TARGET set to '${SNES9X_UPSTREAM_TARGET}' but that target does not exist. Ensure the upstream project defines the target or adjust the variable.")
    endif()
  else()
    if(TARGET snes9x_upstream)
      target_link_libraries(snes9x_core PRIVATE snes9x_upstream)
    else()
      message(FATAL_ERROR "USE_UPSTREAM_CORE is ON but no upstream target 'snes9x_upstream' was found. Set SNES9X_UPSTREAM_TARGET or ensure upstream defines the expected target.")
    endif()
  endif()

  # Propagate HEADLESS_ONLY definition
  target_compile_definitions(snes9x_core PUBLIC HEADLESS_ONLY=$<BOOL:${HEADLESS_ONLY}>)

  enable_language(C)

else()
  # Build core as static or shared depending on option
  if(BUILD_SHARED_CORE)
    add_library(snes9x_core SHARED core_stub.cpp)
  else()
    add_library(snes9x_core STATIC core_stub.cpp)
  endif()

  target_include_directories(snes9x_core
    PUBLIC
      ${CMAKE_SOURCE_DIR}/include
  )

  target_compile_definitions(snes9x_core PUBLIC HEADLESS_ONLY=$<BOOL:${HEADLESS_ONLY}>)

  enable_language(C)

endif()
