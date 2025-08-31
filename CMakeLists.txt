cmake_minimum_required(VERSION 3.20)
project(chip8 C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB CHIP8_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
add_executable(chip8 ${CHIP8_SOURCES})
target_include_directories(chip8 PRIVATE "${CMAKE_SOURCE_DIR}/include")

find_package(SDL3 CONFIG REQUIRED)      # 由 SDL3_DIR 指向的包提供
target_link_libraries(chip8 PRIVATE SDL3::SDL3)

if (WIN32)
  add_custom_command(TARGET chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:chip8> $<TARGET_FILE_DIR:chip8>
    COMMAND_EXPAND_LISTS
    COMMENT "Copy runtime DLLs next to chip8.exe")
endif()

install(TARGETS chip8 RUNTIME DESTINATION bin)
