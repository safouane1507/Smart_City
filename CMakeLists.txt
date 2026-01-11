cmake_minimum_required(VERSION 3.20)

project(parklogic VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Dependencies ---
include(FetchContent)
set(RAYLIB_VERSION 5.5)
message(STATUS "Fetching Raylib ${RAYLIB_VERSION}...")

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP true
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

# This tells CMake that the include directories for the 'raylib' target
# should be treated as SYSTEM headers (suppressing warnings) when used by other targets.
target_include_directories(raylib SYSTEM INTERFACE ${raylib_SOURCE_DIR}/src)

# --- Sources ---
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB ADAPTIVE_SOURCES "adaptive-signals/src/*.cpp")
list(APPEND SOURCES ${ADAPTIVE_SOURCES})
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/systems/TrackingSystem.cpp")
# --- Target ---
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/adaptive-signals/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# --- Assets ---
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory"
)

# --- Compiler Flags ---
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /EHsc)
    # --- Assets ---
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/Debug/assets
        COMMENT "Copying assets to build directory"
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_subdirectory(tests)
