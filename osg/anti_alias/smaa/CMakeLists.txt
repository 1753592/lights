cmake_minimum_required(VERSION 3.8.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(app_name smaa)

set(hdr
	testNode.h
)

set(src
	main.cpp
	testNode.cpp
)

set(libs
	OSG_LIBRARY
	OSGDB_LIBRARY
	OSGUTIL_LIBRARY
	OSGVIEWER_LIBRARY
	OSGGA_LIBRARY
)

add_executable(${app_name} ${src} ${hdr})

target_compile_definitions(${app_name} PRIVATE RS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${app_name}>"
                                               VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${app_name}>"
                                               VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin")

target_include_directories(${app_name} PRIVATE ${OSG_INCLUDE_DIR})
#target_include_directories(${app_name} PRIVATE ${Boost_INCLUDE_DIR})

foreach(lib ${libs})
	target_link_libraries(${app_name} debug ${${lib}_DEBUG} optimized ${${lib}_RELEASE})
endforeach()

set_target_properties(${app_name} PROPERTIES FOLDER "o/anti_alias")

#set(SHADERS
#	../glsl/noise.glsl
#)
#
#source_group(shaders FILES ${SHADERS})
#target_sources(${app_name} PRIVATE ${SHADERS})

