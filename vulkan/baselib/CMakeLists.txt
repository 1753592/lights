cmake_minimum_required(VERSION 3.8.0)

set(prj_name baselib)

set(imgui/hdr
	imgui/imgui_impl_sdl2.h
	imgui/imgui_impl_vulkan.h
)

set(imgui_src
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_impl_sdl2.cpp
	imgui/imgui_impl_vulkan.cpp
)

set(hdr
	VulkanInstance.h
	VulkanSwapChain.h
	VulkanView.h
	VulkanBuffer.h
	VulkanDevice.h
	VulkanDebug.h
	VulkanTools.h
	VulkanInitializers.hpp

	VulkanImGUI.h

	Manipulator.h

	${imgui_hdr}
)

set(src
	VulkanInstance.cpp
	VulkanSwapChain.cpp
	VulkanView.cpp
	VulkanBuffer.cpp
	VulkanDevice.cpp
	VulkanDebug.cpp
	VulkanTools.cpp

	VulkanImGUI.cpp

	Manipulator.cpp

	${imgui_src}
)

source_group("imgui" FILES ${imgui_src} ${imgui_hdr})

find_package(Vulkan)

add_library(${prj_name} ${src} ${hdr})

target_include_directories(${prj_name} PUBLIC ${Vulkan_INCLUDE_DIR})

target_link_libraries(${prj_name} PUBLIC ${Vulkan_LIBRARY})
target_link_libraries(${prj_name} PUBLIC SDL2::SDL2 SDL2::SDL2main)

set_target_properties(${prj_name} PROPERTIES FOLDER "v")

