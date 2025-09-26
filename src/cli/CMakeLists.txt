cmake_minimum_required(VERSION 3.15)

add_executable(snes9x-cli main.cpp)

target_include_directories(snes9x-cli PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(snes9x-cli PRIVATE snes9x_core)

# Install target for convenience
install(TARGETS snes9x-cli RUNTIME DESTINATION bin)
