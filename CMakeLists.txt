project(cbs_basic CXX C)
cmake_minimum_required(VERSION 3.0.0)

find_package(PkgConfig)
pkg_check_modules(YamlCpp yaml-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "/usr/bin/g++-7")
# Pset_target_properties(cbs_basic PROPERTIES LINKER_LANGUAGE CXX)

set(SOURCE_LIST
    AStar.cpp
    agents.cpp
    CBS.cpp
    common.cpp
    graph.cpp
    PBS.cpp
    task_assigner.cpp
    topsort.cpp
)

add_executable(
    PBS
    ${SOURCE_LIST}
    PBS_launch.cpp
)
target_link_libraries(
    PBS
    yaml-cpp
)

add_executable(
    layout_generation
    ${SOURCE_LIST}
    layout_generation_launch.cpp
)
target_link_libraries(
    layout_generation
    yaml-cpp
)
