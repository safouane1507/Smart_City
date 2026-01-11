include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Collect source files (excluding main.cpp)
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB ADAPTIVE_SOURCES "${CMAKE_SOURCE_DIR}/adaptive-signals/src/*.cpp")
list(APPEND TEST_SOURCES ${ADAPTIVE_SOURCES})
list(FILTER TEST_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# Declare the test executable
add_executable(unit_tests
    ${TEST_SOURCES}
    MainTests.cpp
    EventBusTests.cpp
    GameLoopTests.cpp
    SceneManagerTests.cpp
    GameSceneTests.cpp
    WindowTests.cpp
)




target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/adaptive-signals/include
)

target_link_libraries(unit_tests PRIVATE
    GTest::gtest_main
    raylib
)

# --- Assets for Tests ---
add_custom_command(TARGET unit_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_BINARY_DIR}/assets
    COMMENT "Copying assets to tests build directory"
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
