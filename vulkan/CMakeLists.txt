cmake_minimum_required(VERSION 3.8.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan)
find_package(SDL2)

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
function(compile_shader target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT" "SOURCES")
    foreach(source ${arg_SOURCES})
        add_custom_command(
            OUTPUT ${source}.spv
            DEPENDS ${source}
            #DEPFILE ${source}.d
            COMMAND
                ${glslc_executable}
                #$<$<BOOL:${arg_ENV}>:--target-env=${arg_ENV}>
                $<$<BOOL:${arg_FORMAT}>:-mfmt=${arg_FORMAT}>
                #-MD -MF ${source}.d
                -o ${CMAKE_CURRENT_SOURCE_DIR}/${source}.spv
                ${CMAKE_CURRENT_SOURCE_DIR}/${source}
        )
        target_sources(${target} PRIVATE ${source}.spv)
    endforeach()
endfunction()

add_subdirectory(basic_pbr)
