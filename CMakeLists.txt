cmake_minimum_required(VERSION 3.20)
project(chip8 C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------
# Core library: exclude main.c to allow unit testing independently
# -----------------------------
file(GLOB CHIP8_ALL_C CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
list(FILTER CHIP8_ALL_C EXCLUDE REGEX ".*/main\\.c$")
add_library(chip8_core ${CHIP8_ALL_C})
target_include_directories(chip8_core PUBLIC "${CMAKE_SOURCE_DIR}/include")

# -----------------------------
# Executable: entry point only, links to core library + SDL3
# -----------------------------
add_executable(chip8 src/main.c)
target_include_directories(chip8 PRIVATE "${CMAKE_SOURCE_DIR}/include")
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(chip8 PRIVATE chip8_core SDL3::SDL3)

if (WIN32)
  add_custom_command(TARGET chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:chip8> $<TARGET_FILE_DIR:chip8>
    COMMAND_EXPAND_LISTS
    COMMENT "Copy runtime DLLs next to chip8.exe")
endif()

install(TARGETS chip8 RUNTIME DESTINATION bin)

# -----------------------------
# Tests: GoogleTest + CTest
# -----------------------------
include(CTest)           # Enable BUILD_TESTING option (default ON)
if (BUILD_TESTING)
  # Option: use local googletest source
  # The root path should contain the googletest/ subdir and top-level CMakeLists.txt
  set(GTEST_SRC_ROOT "C:/Libraries/googletest-1.17.0" CACHE PATH "Path to googletest source root")
  add_subdirectory("${GTEST_SRC_ROOT}" "${CMAKE_BINARY_DIR}/_gtest")   # Generates gtest / gtest_main targets

  add_executable(test_mem tests/test_mem.cpp)
  target_include_directories(test_mem PRIVATE "${CMAKE_SOURCE_DIR}/include")
  target_link_libraries(test_mem PRIVATE chip8_core gtest gtest_main)

  include(GoogleTest)
  gtest_discover_tests(test_mem)   # Automatically register TEST() cases into CTest
endif()
