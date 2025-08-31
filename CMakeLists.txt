cmake_minimum_required(VERSION 3.20)
project(chip8 C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# err logging
option(CHIP8_ENABLE_LOG "Enable Chip8 logging to stderr" OFF)

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

# keyboard err logging
if (CHIP8_ENABLE_LOG)
  target_compile_definitions(chip8_core PUBLIC CHIP8_ENABLE_LOG)
endif()

install(TARGETS chip8 RUNTIME DESTINATION bin)

# -----------------------------
# Tests: GoogleTest + CTest (auto-discover tests/tests_*.cpp)
# -----------------------------
include(CTest)  # Enable BUILD_TESTING option (default ON)
if (BUILD_TESTING)
  # Use local googletest sources (the path must contain the "googletest/" subdir and a top-level CMakeLists.txt)
  set(GTEST_SRC_ROOT "C:/Libraries/googletest-1.17.0" CACHE PATH "Path to googletest source root")
  add_subdirectory("${GTEST_SRC_ROOT}" "${CMAKE_BINARY_DIR}/_gtest")  # Provides gtest / gtest_main

  # Collect all test sources under tests/ matching test_*.cpp
  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/test_*.cpp")

  if (TEST_SOURCES STREQUAL "")
    message(WARNING "No test sources found in ${CMAKE_SOURCE_DIR}/tests/test_*.cpp")
  endif()

  include(GoogleTest)

  foreach(test_src IN LISTS TEST_SOURCES)
    # Derive target name from filename (e.g., tests/test_mem.cpp -> test_mem)
    get_filename_component(test_name "${test_src}" NAME_WE)

    add_executable(${test_name} "${test_src}")
    target_include_directories(${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/include")
    target_link_libraries(${test_name} PRIVATE chip8_core gtest gtest_main)

    # Register with CTest and let GoogleTest discover TEST() cases automatically
    gtest_discover_tests(${test_name}
      WORKING_DIRECTORY $<TARGET_FILE_DIR:${test_name}>
      DISCOVERY_MODE PRE_TEST)

    # Optional: also add a direct CTest entry for the executable name
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()
endif()

