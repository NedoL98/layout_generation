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
    task_assigner.cpp
)

add_executable(
    cbs_basic
    ${SOURCE_LIST}
    main.cpp
)
target_link_libraries(
    cbs_basic
    yaml-cpp
)
