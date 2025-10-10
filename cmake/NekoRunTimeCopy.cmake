include_guard()

#[=[
    NekoRunTimeCopy - Automatic runtime library copying for cross-platform builds
    
    This module provides functionality to automatically copy runtime libraries
    (DLLs on Windows, shared libraries on Unix-like systems) to the target
    executable directory.
    
    Usage:
        include(cmake/NekoRuntimeCopy.cmake)
        NekoRunTimeCopy(target_name)
        
    Parameters:
        target_name - The name of the target executable that needs runtime libraries
        
    Supported Platforms:
        - Windows (via vcpkg)
        - macOS (via vcpkg or system libraries)
        - Linux (via vcpkg or system libraries)
    
    Environment Variables:
        VCPKG_ROOT - Path to vcpkg installation (optional, will try to detect)
#]=]

function(NekoRunTimeCopy TARGET)
    if(WIN32)
        _neko_copy_runtime_windows(${TARGET})
    elseif(APPLE)
        _neko_copy_runtime_macos(${TARGET})
    elseif(UNIX)
        _neko_copy_runtime_linux(${TARGET})
    else()
        message(WARNING "NekoRunTimeCopy: Unsupported platform")
    endif()
endfunction()

# ============================================================================
# Windows Runtime Copy
# ============================================================================
function(_neko_copy_runtime_windows target)
    # Detect vcpkg root
    if(DEFINED ENV{VCPKG_ROOT})
        set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    elseif(DEFINED CMAKE_TOOLCHAIN_FILE)
        get_filename_component(VCPKG_ROOT "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_ROOT}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_ROOT}" DIRECTORY)
    elseif(EXISTS "C:/dev/vcpkg")
        set(VCPKG_ROOT "C:/dev/vcpkg")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg")
        set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")
    endif()

    if(NOT VCPKG_ROOT)
        message(WARNING "NekoRunTimeCopy: VCPKG_ROOT not found. Runtime DLLs may need to be copied manually.")
        return()
    endif()

    # Detect architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VCPKG_ARCH "x64-windows")
    else()
        set(VCPKG_ARCH "x86-windows")
    endif()

    # Determine the binary directory based on build configuration
    set(VCPKG_BIN_DIR_DEBUG "${VCPKG_ROOT}/installed/${VCPKG_ARCH}/debug/bin")
    set(VCPKG_BIN_DIR_RELEASE "${VCPKG_ROOT}/installed/${VCPKG_ARCH}/bin")
    
    # List of DLLs to copy (name without .dll, debug_suffix)
    set(DLL_LIST
        # libcurl
        "libcurl;-d"
        # OpenSSL
        "libssl-3-x64;"
        "libcrypto-3-x64;"
        # zlib (dependency of libcurl)
        "zlib1;d1"
        # Additional common dependencies
        "brotlicommon;-d"
        "brotlidec;-d"
        "brotlienc;-d"
        "nghttp2;-d"
        "libssh2;-d"
        "idn2-0;"
        "unistring-5;"
        "charset-1;"
        "iconv-2;"
        "intl-8;"
    )
    
    # Build copy commands
    set(COPY_COMMANDS "")
    foreach(dll_entry ${DLL_LIST})
        string(REPLACE ";" "|" dll_parts "${dll_entry}")
        string(REPLACE "|" ";" dll_parts "${dll_parts}")
        list(GET dll_parts 0 dll_base)
        list(LENGTH dll_parts dll_parts_len)
        if(dll_parts_len GREATER 1)
            list(GET dll_parts 1 dll_debug_suffix)
        else()
            set(dll_debug_suffix "")
        endif()
        
        set(DEBUG_DLL "${dll_base}${dll_debug_suffix}.dll")
        set(RELEASE_DLL "${dll_base}.dll")
        
        list(APPEND COPY_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${VCPKG_BIN_DIR_DEBUG}/${DEBUG_DLL},${VCPKG_BIN_DIR_RELEASE}/${RELEASE_DLL}>"
                "$<TARGET_FILE_DIR:${target}>/$<IF:$<CONFIG:Debug>,${DEBUG_DLL},${RELEASE_DLL}>"
                || (exit 0)
        )
    endforeach()
    
    # Add custom command to copy DLLs
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs for $<CONFIG> configuration to $<TARGET_FILE_DIR:${target}>"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>"
        ${COPY_COMMANDS}
        COMMENT "NekoRunTimeCopy: Copying Windows runtime DLLs"
        VERBATIM
    )
    
    message(STATUS "NekoRunTimeCopy: Configured for Windows (${VCPKG_ARCH})")
    message(STATUS "  Debug DLLs from: ${VCPKG_BIN_DIR_DEBUG}")
    message(STATUS "  Release DLLs from: ${VCPKG_BIN_DIR_RELEASE}")
endfunction()

# ============================================================================
# macOS Runtime Copy
# ============================================================================
function(_neko_copy_runtime_macos target)
    # Detect vcpkg root
    if(DEFINED ENV{VCPKG_ROOT})
        set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    elseif(DEFINED CMAKE_TOOLCHAIN_FILE)
        get_filename_component(VCPKG_ROOT "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_ROOT}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_ROOT}" DIRECTORY)
    endif()

    if(NOT VCPKG_ROOT)
        # On macOS, system libraries are usually handled by dyld
        message(STATUS "NekoRunTimeCopy: vcpkg not detected on macOS, using system libraries")
        return()
    endif()

    # Detect architecture
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(VCPKG_ARCH "arm64-osx")
    else()
        set(VCPKG_ARCH "x64-osx")
    endif()

    set(VCPKG_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_ARCH}/lib")
    
    if(NOT EXISTS "${VCPKG_LIB_DIR}")
        message(STATUS "NekoRunTimeCopy: vcpkg lib directory not found: ${VCPKG_LIB_DIR}")
        return()
    endif()

    # List of dylibs to copy
    set(DYLIB_PATTERNS
        "libcurl*.dylib"
        "libssl*.dylib"
        "libcrypto*.dylib"
        "libz*.dylib"
        "libnghttp2*.dylib"
        "libssh2*.dylib"
        "libbrotli*.dylib"
        "libidn2*.dylib"
    )

    # Find and copy dylibs
    foreach(pattern ${DYLIB_PATTERNS})
        file(GLOB dylib_files "${VCPKG_LIB_DIR}/${pattern}")
        foreach(dylib ${dylib_files})
            get_filename_component(dylib_name "${dylib}" NAME)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${dylib}"
                    "$<TARGET_FILE_DIR:${target}>/${dylib_name}"
                COMMENT "Copying ${dylib_name}"
                VERBATIM
            )
        endforeach()
    endforeach()

    # Set RPATH for the target to find the copied libraries
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "$<TARGET_FILE_DIR:${target}>"
        INSTALL_RPATH "@executable_path"
        MACOSX_RPATH TRUE
    )

    message(STATUS "NekoRunTimeCopy: Configured for macOS (${VCPKG_ARCH})")
    message(STATUS "  Libraries from: ${VCPKG_LIB_DIR}")
endfunction()

# ============================================================================
# Linux Runtime Copy
# ============================================================================
function(_neko_copy_runtime_linux target)
    # Detect vcpkg root
    if(DEFINED ENV{VCPKG_ROOT})
        set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    elseif(DEFINED CMAKE_TOOLCHAIN_FILE)
        get_filename_component(VCPKG_ROOT "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_ROOT}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_ROOT}" DIRECTORY)
    endif()

    if(NOT VCPKG_ROOT)
        # On Linux, system libraries are usually handled by ld
        message(STATUS "NekoRunTimeCopy: vcpkg not detected on Linux, using system libraries")
        return()
    endif()

    # Detect architecture
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(VCPKG_ARCH "arm64-linux")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
        set(VCPKG_ARCH "arm-linux")
    else()
        set(VCPKG_ARCH "x64-linux")
    endif()

    set(VCPKG_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_ARCH}/lib")
    
    if(NOT EXISTS "${VCPKG_LIB_DIR}")
        message(STATUS "NekoRunTimeCopy: vcpkg lib directory not found: ${VCPKG_LIB_DIR}")
        return()
    endif()

    # List of shared libraries to copy
    set(SO_PATTERNS
        "libcurl.so*"
        "libssl.so*"
        "libcrypto.so*"
        "libz.so*"
        "libnghttp2.so*"
        "libssh2.so*"
        "libbrotlicommon.so*"
        "libbrotlidec.so*"
        "libbrotlienc.so*"
        "libidn2.so*"
    )

    # Find and copy shared libraries
    set(COPY_COMMANDS "")
    foreach(pattern ${SO_PATTERNS})
        file(GLOB so_files "${VCPKG_LIB_DIR}/${pattern}")
        foreach(so_file ${so_files})
            get_filename_component(so_name "${so_file}" NAME)
            list(APPEND COPY_COMMANDS
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${so_file}"
                    "$<TARGET_FILE_DIR:${target}>/${so_name}"
                    || (exit 0)
            )
        endforeach()
    endforeach()

    if(COPY_COMMANDS)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime shared libraries to $<TARGET_FILE_DIR:${target}>"
            ${COPY_COMMANDS}
            COMMENT "NekoRunTimeCopy: Copying Linux runtime libraries"
            VERBATIM
        )
    endif()

    # Set RPATH for the target to find the copied libraries
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "$<TARGET_FILE_DIR:${target}>:${VCPKG_LIB_DIR}"
        INSTALL_RPATH "$ORIGIN:${VCPKG_LIB_DIR}"
    )

    message(STATUS "NekoRunTimeCopy: Configured for Linux (${VCPKG_ARCH})")
    message(STATUS "  Libraries from: ${VCPKG_LIB_DIR}")
endfunction()