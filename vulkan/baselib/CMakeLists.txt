cmake_minimum_required(VERSION 3.8.0)

set(prj_name baselib)

set(imgui/hdr
)

set(imgui_src
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
)

set(hdr
	VulkanInstance.h
	VulkanSwapChain.h
	VulkanView.h
	VulkanBuffer.h
	VulkanDevice.h
	VulkanTexture.h
	VulkanDebug.h
	VulkanTools.h
	VulkanInitializers.hpp

	MeshInstance.h
	MeshPrimitive.h

	VulkanImGUI.h

	Manipulator.h

	GLTFLoader.h

	${imgui_hdr}
)

set(src
	VulkanInstance.cpp
	VulkanSwapChain.cpp
	VulkanView.cpp
	VulkanBuffer.cpp
	VulkanDevice.cpp
	VulkanTexture.cpp
	VulkanDebug.cpp
	VulkanTools.cpp

	MeshInstance.cpp
	MeshPrimitive.cpp

	VulkanImGUI.cpp

	Manipulator.cpp

	GLTFLoader.cpp

	${imgui_src}
)

source_group("imgui" FILES ${imgui_src} ${imgui_hdr})

find_package(Vulkan)

add_library(${prj_name} ${src} ${hdr})


set(shaders
	shaders/pbr_clr.vert
	shaders/pbr_tex.vert
	shaders/pbr_clr.frag
	shaders/pbr_tex.frag
)

source_group(shaders FILES ${shaders})
target_sources(${prj_name} PRIVATE ${shaders})

target_include_directories(${prj_name} PUBLIC ${Vulkan_INCLUDE_DIR})

target_link_libraries(${prj_name} PUBLIC ${Vulkan_LIBRARY})
target_link_libraries(${prj_name} PUBLIC SDL2::SDL2 SDL2::SDL2main)

set_target_properties(${prj_name} PROPERTIES FOLDER "v")

compile_shader(${prj_name} ENV opengl FORMAT bin SOURCES ${shaders})

