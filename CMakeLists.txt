
cmake_minimum_required(VERSION 3.8.0)

project(lights)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

option(build_opengl "" ON)
option(build_vulkan "" ON)

configure_file(config config.h)

include_directories(${CMAKE_BINARY_DIR})

if(build_opengl)
  add_subdirectory(osg)
endif()

if(build_vulkan)
  add_subdirectory(vulkan)
endif()

