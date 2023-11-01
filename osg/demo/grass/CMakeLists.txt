cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(app_name grass)

set(hdr
	testNode.h
)

set(src
	main.cpp
	testNode.cpp
)

set(libs
	OPENTHREADS_LIBRARY
	OSG_LIBRARY
	OSGDB_LIBRARY
	OSGUTIL_LIBRARY
	OSGVIEWER_LIBRARY
	OSGGA_LIBRARY
)

add_executable(${app_name} ${src} ${hdr})

set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${app_name}>"
                                               VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${app_name}>"
                                               VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${osg_build_dir}/bin")

target_include_directories(${app_name} PRIVATE ${OSG_INCLUDE_DIR})

foreach(lib ${libs})
	#target_link_libraries(${app_name} debug ${${lib}_DEBUG} optimized ${${lib}_RELEASE})
	target_link_libraries(${app_name} PRIVATE ${${lib}})
endforeach()

set_target_properties(${app_name} PROPERTIES FOLDER "o/demos")
