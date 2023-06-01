cmake_minimum_required(VERSION 3.8.0)



set(app_name basic_pbr)

set(hdr
)

set(src
	main.cpp
)

add_executable(${app_name} ${src} ${hdr})

set(SHADERS
	pbr.vert
	pbr.frag
)

source_group(shaders FILES ${SHADERS})
target_sources(${app_name} PRIVATE ${SHADERS})


set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                                               VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${app_name}>"
                                               VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin"
											   VS_DEBUGGER_COMMAND_ARGUMENTS "")

foreach(lib ${libs})
	target_link_libraries(${app_name} debug ${${lib}_DEBUG} optimized ${${lib}_RELEASE})
endforeach()

target_include_directories(${app_name} PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(${app_name} PRIVATE ${CMAKE_SOURCE_DIR}/vulkan/baselib)

target_link_libraries(${app_name} ${Vulkan_LIBRARY})
target_link_libraries(${app_name} SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(${app_name} baselib)


set_target_properties(${app_name} PROPERTIES FOLDER "v")


compile_shader(${app_name} ENV opengl FORMAT bin SOURCES ${SHADERS})

