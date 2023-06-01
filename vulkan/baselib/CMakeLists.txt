cmake_minimum_required(VERSION 3.8.0)

set(app_name baselib)

set(hdr
	VulkanInstance.h
	VulkanView.h
	VulkanBuffer.h
	VulkanDevice.h
	VulkanDebug.h
)

set(src
	VulkanInstance.cpp
	VulkanView.cpp
	VulkanBuffer.cpp
	VulkanDevice.cpp
	VulkanDebug.cpp
)

find_package(Vulkan)

add_library(${app_name} ${src} ${hdr})

target_include_directories(${app_name} PUBLIC ${Vulkan_INCLUDE_DIR})

target_link_libraries(${app_name} PUBLIC ${Vulkan_LIBRARY})
target_link_libraries(${app_name} PUBLIC SDL2::SDL2 SDL2::SDL2main)

set_target_properties(${app_name} PROPERTIES FOLDER "v")

