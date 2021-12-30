cmake_minimum_required(VERSION 3.5)
project(wgpu-starter)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ExternalProject)

include(cmake/glm.cmake)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

set(CMAKE_EXE_LINKER_FLAGS "-s USE_WEBGPU=1")
# Generate the index.html file that will load our Emscripten compiled module
set(APP_TARGET_NAME wgpu-starter)
configure_file(index.html.in ${CMAKE_CURRENT_BINARY_DIR}/index.html @ONLY)

add_executable(wgpu-starter
    main.cpp
    arcball_camera.cpp)

set_target_properties(wgpu-starter PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_link_libraries(wgpu-starter PRIVATE glm)
