if(WGPU_NATIVE_DIR)
    set(TARGET_NAME dawn)

    add_library(Dawn INTERFACE)
    
    if(MSVC)
        add_definitions(-DWGPU_TARGET=WGPU_TARGET_WINDOWS)
        set(OS_LIBRARIES "userenv" "ws2_32" "Dwmapi" "dbghelp" "d3dcompiler" "D3D12" "D3D11" "DXGI" "setupapi" "Bcrypt")
    elseif(APPLE)
        add_definitions(-DWGPU_TARGET=WGPU_TARGET_MACOS)
        set(OS_LIBRARIES "-framework Cocoa" "-framework CoreVideo" "-framework IOKit" "-framework QuartzCore")
        #target_compile_options(webgpu PRIVATE -x objective-c)
    else(MSVC)
        if(USE_WAYLAND)
            add_definitions(-DWGPU_TARGET=WGPU_TARGET_LINUX_WAYLAND)
        else(USE_WAYLAND)
            add_definitions(-DWGPU_TARGET=WGPU_TARGET_LINUX_X11)
        endif(USE_WAYLAND)
    endif(MSVC)

    find_library(WGPU_LIBRARY wgpu_native
        HINTS "${WGPU_NATIVE_DIR}/target/debug"
    )

    find_path(WGPU_NATIVE_INCLUDE_DIR
        NAME wgpu.h
        PATHS
        ${WGPU_NATIVE_DIR}/ffi/
    )
    mark_as_advanced(WGPU_NATIVE_INCLUDE_DIR)

    add_library(wgpunative UNKNOWN IMPORTED)
    set_target_properties(wgpunative PROPERTIES
        IMPORTED_LOCATION
        ${WGPU_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES
        "${WGPU_NATIVE_INCLUDE_DIR}")
        
    target_link_libraries(Dawn INTERFACE wgpunative ${OS_LIBRARIES})
else()
    find_path(DAWN_WGPU_INCLUDE_DIR
        NAME dawn/webgpu.h
        PATHS
        ${Dawn_DIR}/gen/include/
    )
    mark_as_advanced(DAWN_WGPU_INCLUDE_DIR)

    find_path(DAWN_INCLUDE_DIR
        NAME dawn/EnumClassBitmasks.h
        PATHS
        ${Dawn_DIR}/../../include/
    )
    mark_as_advanced(DAWN_INCLUDE_DIR)

    set(DAWN_INCLUDE_DIRS ${DAWN_WGPU_INCLUDE_DIR} ${DAWN_INCLUDE_DIR})

    find_library(DAWN_NATIVE_LIBRARY
        NAMES
        dawn_native
        # On windows Dawn has the dll in the name
        dawn_native.dll.lib
        libdawn_native.so
        PATHS ${Dawn_DIR}
        NO_DEFAULT_PATH
    )

    find_library(DAWN_PLATFORM_LIBRARY
        NAMES
        dawn_platform
        # On windows Dawn has the dll in the name
        dawn_platform.dll.lib
        libdawn_platform.so
        PATHS ${Dawn_DIR}
        NO_DEFAULT_PATH
    )

    find_library(DAWN_PROC_LIBRARY
        NAMES
        dawn_proc
        # On windows Dawn has the dll in the name
        dawn_proc.dll.lib
        libdawn_proc.so
        PATHS ${Dawn_DIR}
        NO_DEFAULT_PATH
    )

    set(DAWN_LIBRARIES
        ${DAWN_NATIVE_LIBRARY}
        ${DAWN_PLATFORM_LIBRARY}
        ${DAWN_PROC_LIBRARY})

    include(FindPackageHandleStandardArgs)

    find_package_handle_standard_args(Dawn REQUIRED_VARS
        DAWN_INCLUDE_DIR
        DAWN_WGPU_INCLUDE_DIR
        DAWN_NATIVE_LIBRARY
        DAWN_PLATFORM_LIBRARY
        DAWN_PROC_LIBRARY)

    if (Dawn_FOUND AND NOT TARGET Dawn)
        add_library(Dawn::Native UNKNOWN IMPORTED)
        set_target_properties(Dawn::Native PROPERTIES
            IMPORTED_LOCATION
            ${DAWN_NATIVE_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES
            "${DAWN_INCLUDE_DIRS}")
        target_compile_features(Dawn::Native INTERFACE cxx_std_11)

        add_library(Dawn::Platform UNKNOWN IMPORTED)
        set_target_properties(Dawn::Platform PROPERTIES
            IMPORTED_LOCATION
            ${DAWN_PLATFORM_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES
            "${DAWN_INCLUDE_DIRS}")
        target_compile_features(Dawn::Platform INTERFACE cxx_std_11)

        add_library(Dawn::Proc UNKNOWN IMPORTED)
        set_target_properties(Dawn::Proc PROPERTIES
            IMPORTED_LOCATION
            ${DAWN_PROC_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES
            "${DAWN_INCLUDE_DIRS}")
        target_compile_features(Dawn::Proc INTERFACE cxx_std_11)

        add_library(Dawn INTERFACE)
        target_compile_features(Dawn INTERFACE cxx_std_11)
        target_link_libraries(Dawn INTERFACE
            Dawn::Native
            Dawn::Platform
            Dawn::Proc)
    endif()
endif()