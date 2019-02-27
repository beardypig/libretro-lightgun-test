cmake_minimum_required(VERSION  3.7)

project(lightgun_test VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# custom options for MacOS
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/src/link.T")

set(SOURCE_FILES src/libretro.cpp src/sysfont.cpp)

add_library(lightgun_test_libretro SHARED ${SOURCE_FILES})
set_target_properties(lightgun_test_libretro PROPERTIES PREFIX "")  # remove lib prefix from target
