
cmake_minimum_required(VERSION 3.8.0)


set(CMAKE_PREFIX_PATH)

option(build_shadowmap "build_shadowmap" ON)

if(build_shadowmap)
add_subdirectory(basic_shadowmap)
add_subdirectory(perspective_shadowmap)
endif()

