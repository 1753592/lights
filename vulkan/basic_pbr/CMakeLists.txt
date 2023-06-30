cmake_minimum_required(VERSION 3.8.0)



set(prj_name basic_pbr)

set(hdr
)

set(src
	main.cpp
)

add_executable(${prj_name} ${src} ${hdr})

set(shaders
	pbr.vert
	pbr.frag
	#test.vert
	#test.frag
)

source_group(shaders FILES ${shaders})
target_sources(${prj_name} PRIVATE ${shaders})


set_target_properties(${prj_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                                               VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${prj_name}>"
                                               VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin"
											   VS_DEBUGGER_COMMAND_ARGUMENTS "")

foreach(lib ${libs})
	target_link_libraries(${prj_name} debug ${${lib}_DEBUG} optimized ${${lib}_RELEASE})
endforeach()

target_include_directories(${prj_name} PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(${prj_name} PRIVATE ${CMAKE_SOURCE_DIR}/vulkan/baselib)

target_link_libraries(${prj_name} ${Vulkan_LIBRARY})
target_link_libraries(${prj_name} SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(${prj_name} baselib)


set_target_properties(${prj_name} PROPERTIES FOLDER "v")

compile_shader(${prj_name} ENV opengl FORMAT bin SOURCES ${shaders})

#target_compile_definitions(${prj_name} PRIVATE SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
