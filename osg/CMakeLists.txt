
cmake_minimum_required(VERSION 3.8.0)


set(CMAKE_PREFIX_PATH)


find_package(osg)
find_package(osgDB)
find_package(osgViewer)
find_package(osgUtil)
find_package(osgGA)

add_subdirectory(basic-lighting)
add_subdirectory(basic-material)
add_subdirectory(pbr-lighting)
add_subdirectory(pbr-lighting-tex)
add_subdirectory(pbr-ibl)
add_subdirectory(pbr-ibl-lighting)
add_subdirectory(pbr-ibl-lighting-rfl)
add_subdirectory(pbr-ibl-lighting-tex)

add_subdirectory(demo)
