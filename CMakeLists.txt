
cmake_minimum_required(VERSION 3.8.0)

project(lights)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_OPENGL "" ON)
option(BUILD_VULKAN "" ON)

configure_file(config config.h)

include_directories(${CMAKE_BINARY_DIR})

if(BUILD_OPENGL)
  add_subdirectory(osg)
endif()

if(BUILD_VULKAN)
  add_subdirectory(vulkan)
endif()

