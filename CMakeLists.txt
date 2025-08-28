cmake_minimum_required(VERSION 3.20)
project(chip8 C)
set(CMAKE_C_STANDARD 11)

# vs code highlight
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===== Default build type (for single-config generators like Ninja) =====
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# ===== SDL3 install root (can be root or its 'cmake/' subdir) =====
# Example (PowerShell):  $env:SDL3_DIR='C:/Libraries/SDL3-3.2.20'
set(SDL3_DIR "$ENV{SDL3_DIR}" CACHE PATH "SDL3 root (contains cmake/, include/, lib/)")
if(NOT SDL3_DIR)
  message(FATAL_ERROR "Please set SDL3_DIR to your SDL3 install root.")
endif()

# Extend package search path (does not modify SDL3_DIR itself)
list(PREPEND CMAKE_PREFIX_PATH "${SDL3_DIR}" "${SDL3_DIR}/cmake")

# ===== Target architecture (auto-detect; user can override with -DSDL_ARCH=...) =====
if(NOT DEFINED SDL_ARCH OR SDL_ARCH STREQUAL "")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "[Aa][Rr][Mm]64|aarch64")
    set(_SDL_ARCH_DEFAULT "arm64")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_SDL_ARCH_DEFAULT "x64")
  else()
    set(_SDL_ARCH_DEFAULT "x86")
  endif()
  set(SDL_ARCH "${_SDL_ARCH_DEFAULT}")
endif()

# Put SDL_ARCH into cache, then mark allowed values
set(SDL_ARCH "${SDL_ARCH}" CACHE STRING "Target architecture" FORCE)
set_property(CACHE SDL_ARCH PROPERTY STRINGS x86 x64 arm64)

# ===== Sources =====
file(GLOB CHIP8_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
add_executable(chip8 ${CHIP8_SOURCES})
target_include_directories(chip8 PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Output paths: exe -> bin/<Config>; intermediates under build/
set_target_properties(chip8 PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib"
)

# ===== Find SDL3 (Config package from your install) --NOT WORKING!! =====
find_package(SDL3 REQUIRED CONFIG)

# 显式包含 SDL3 的头文件目录 (避免 SDL3_INC_DIRS-NOTFOUND)
target_include_directories(chip8 PRIVATE "${SDL3_DIR}/include")

# 链接 SDL3
target_link_libraries(chip8 PRIVATE SDL3::SDL3)

# Resolve actual file paths from the imported target (robust)
set(SDL3_DLL_GEN    "$<TARGET_FILE:SDL3::SDL3>")          # e.g. .../SDL3.dll
set(SDL3_IMPLIB_GEN "$<TARGET_LINKER_FILE:SDL3::SDL3>")   # e.g. .../SDL3.lib
set(STAGE_LIB_DIR   "${CMAKE_SOURCE_DIR}/lib/${SDL_ARCH}")

# ===== Stage SDL3 files into lib/<arch> =====
add_custom_target(stage_sdl_to_lib ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${STAGE_LIB_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_IMPLIB_GEN}" "${STAGE_LIB_DIR}/SDL3.lib"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_DLL_GEN}"    "${STAGE_LIB_DIR}/SDL3.dll"
  BYPRODUCTS "${STAGE_LIB_DIR}/SDL3.lib" "${STAGE_LIB_DIR}/SDL3.dll"
  COMMENT "Staging SDL3 import lib & dll into lib/${SDL_ARCH}"
)
add_dependencies(chip8 stage_sdl_to_lib)

# ===== After build: copy SDL3.dll next to the exe =====
add_custom_command(TARGET chip8 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_DLL_GEN}" "$<TARGET_FILE_DIR:chip8>/SDL3.dll"
  COMMENT "Copy SDL3.dll next to the executable"
)

# ===== Convenience clean targets =====
# Wipe contents of bin/ and build/ (keep directories)
add_custom_target(clean-extra
  COMMAND ${CMAKE_COMMAND} -E echo "Cleaning bin/ and build/ contents..."
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/bin/*"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/*"
  COMMENT "clean-extra done."
)

# Remove bin/ and lib/, and wipe build/ contents (keep build/ dir to avoid deleting the active build tree)
add_custom_target(distclean
  COMMAND ${CMAKE_COMMAND} -E echo "Distclean: removing bin/, lib/, and wiping build/ contents..."
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/bin"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/lib"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/*"
  COMMENT "distclean done (build/ directory kept)."
)
