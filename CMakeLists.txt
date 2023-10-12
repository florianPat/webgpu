cmake_minimum_required(VERSION 3.20)
project(webgpu)

# set default build type if none is selected
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
	message(STATUS "No build type selected, default to Debug")
	set(CMAKE_BUILD_TYPE "Debug")
endif()

# --- Add executable ----
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
add_executable(webgpu
    		   src/main.cpp
			   src/Utils.cpp)
set_target_properties(webgpu PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)
# -----------------------

# ---- Add dawn library ----
option(DAWN_USE_GLFW OFF)
option(DAWN_BUILD_SAMPLES OFF)
option(TINT_BUILD_DOCS OFF)
add_subdirectory(dawn)
# --------------------------

if (NOT EMSCRIPTEN)
	target_compile_definitions(webgpu
							   PRIVATE
							   NOMINMAX WIN32_LEAN_AND_MEAN)
	find_package(Threads REQUIRED)

	target_link_libraries(webgpu PUBLIC
						  dawncpp
						  dawn_native
						  dawn_platform
						  dawn_proc)

	if (WIN32)
	elseif(APPLE)
	else()
		target_compile_definitions(webgpu
								   PRIVATE
								   LINUX)
		find_package(X11 REQUIRED)
		find_package(OpenGL REQUIRED EGL GLX OpenGL)
		target_link_libraries(webgpu
							  PUBLIC
							  X11
							  OpenGL::GL OpenGL::EGL OpenGL::GLX)
	endif()
else()
	set(CMAKE_EXE_LINKER_FLAGS "-s USE_WEBGPU=1")
	# Generate the index.html file that will load our Emscripten compiled module
	set(APP_TARGET_NAME webgpu)
	configure_file(index.html.in ${CMAKE_CURRENT_BINARY_DIR}/index.html @ONLY)
endif()
