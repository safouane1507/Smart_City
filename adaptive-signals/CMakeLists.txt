cmake_minimum_required(VERSION 3.10)
project(TrafficControlSystem)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB SOURCES
    "main.cpp"
    "src/*.cpp"
)


list(FILTER SOURCES EXCLUDE REGEX "visualisation_raylib.cpp")
list(FILTER SOURCES EXCLUDE REGEX "visualisation_raylib.cpp.old")


include_directories(.)
include_directories(include)


add_executable(${PROJECT_NAME} ${SOURCES})


target_link_libraries(${PROJECT_NAME}
    raylib
    GL
    m
    pthread
    dl
    rt
    X11
)
