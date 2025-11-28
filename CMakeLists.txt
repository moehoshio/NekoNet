cmake_minimum_required(VERSION 3.15)
project(NekoNetwork VERSION 1.0.0 LANGUAGES CXX)

# ================
# ==== Config ====
# ================

option(NEKO_NETWORK_AUTO_FETCH_DEPS "Neko Network Automatically fetch dependencies" ON)
option(NEKO_NETWORK_BUILD_TESTS "Neko Network Build tests" ON)
option(NEKO_NETWORK_STATIC_LINK "Neko Network Static Link library" OFF)

set(NEKO_NETWORK_LIBRARY_PATH "" CACHE PATH "Path to look for dependencies (OpenSSL, libcurl, GTest)")

# Set MSVC runtime library to match vcpkg triplet
# vcpkg's x64-windows-static uses /MT (static runtime)
# vcpkg's x64-windows uses /MD (dynamic runtime)
if(MSVC)
    if(VCPKG_TARGET_TRIPLET)
        if(VCPKG_TARGET_TRIPLET MATCHES ".*-static$")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
            message(STATUS "Using static MSVC runtime (/MT) for triplet: ${VCPKG_TARGET_TRIPLET}")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
            message(STATUS "Using dynamic MSVC runtime (/MD) for triplet: ${VCPKG_TARGET_TRIPLET}")
        endif()
    endif()
endif()

if(NEKO_NETWORK_LIBRARY_PATH)
    list(APPEND CMAKE_PREFIX_PATH ${NEKO_NETWORK_LIBRARY_PATH})
endif(NEKO_NETWORK_LIBRARY_PATH)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(NekoSchema QUIET)
find_package(NekoFunction QUIET)
find_package(NekoSystem QUIET)
find_package(NekoLog QUIET)

find_package(OpenSSL QUIET)
find_package(CURL QUIET)
find_package(GTest QUIET)

# Detect CURL SSL backend
set(CURL_USES_SCHANNEL FALSE)
set(CURL_SSL_BACKEND "Unknown")

if(CURL_FOUND)
    # Check if CURL is using Schannel (Windows native SSL)
    if(WIN32)
        # On Windows, vcpkg typically builds curl with Schannel
        if(CURL_INCLUDE_DIRS MATCHES "vcpkg" OR NOT OPENSSL_FOUND)
            set(CURL_USES_SCHANNEL TRUE)
            set(CURL_SSL_BACKEND "Schannel (Windows native)")
        elseif(OPENSSL_FOUND)
            set(CURL_SSL_BACKEND "OpenSSL")
        endif()
    else()
        # On Unix, typically OpenSSL
        set(CURL_SSL_BACKEND "OpenSSL")
    endif()
endif()

# Print configuration summary
message(STATUS "Start configuration Neko Network...")
message(STATUS "")
message(STATUS "NekoNetwork configuration summary:")
message(STATUS "")
message(STATUS "  - CMake version: ${CMAKE_VERSION}")
message(STATUS "  - Neko Network Library Path: ${NEKO_NETWORK_LIBRARY_PATH}")
message(STATUS "  - CMake Prefix Path: ${CMAKE_PREFIX_PATH}")
message(STATUS "")
message(STATUS "  - Neko Network Auto fetch deps: ${NEKO_NETWORK_AUTO_FETCH_DEPS}")
message(STATUS "  - Neko Network Build tests: ${NEKO_NETWORK_BUILD_TESTS}")
message(STATUS "  - Neko Network Static Link library: ${NEKO_NETWORK_STATIC_LINK}")
message(STATUS "")
message(STATUS "Dependency summary:")
message(STATUS "  - NekoSchema : ${NekoSchema_FOUND} version : ${NekoSchema_VERSION}")
message(STATUS "  - NekoFunction : ${NekoFunction_FOUND} version : ${NekoFunction_VERSION}")
message(STATUS "  - NekoSystem : ${NekoSystem_FOUND} version : ${NekoSystem_VERSION}")
message(STATUS "  - NekoLog : ${NekoLog_FOUND} version : ${NekoLog_VERSION}")
message(STATUS "  - OpenSSL support: ${OPENSSL_FOUND} version: ${OPENSSL_VERSION}")
message(STATUS "  - libcurl support: ${CURL_FOUND} version: ${CURL_VERSION_STRING}")
message(STATUS "  - CURL SSL backend: ${CURL_SSL_BACKEND}")
message(STATUS "  - GTest : ${GTest_FOUND} version : ${GTest_VERSION}")
message(STATUS "")

if (NOT CURL_FOUND)
    message(FATAL_ERROR "Neko Network not found libcurl ; if you installed it, you may need to set -DNEKO_NETWORK_LIBRARY_PATH=<path>")
endif()

# On Windows, OpenSSL is optional if using Schannel (native SSL)
# On other platforms, OpenSSL is required
if (NOT WIN32 AND NOT OPENSSL_FOUND)
    message(FATAL_ERROR "Neko Network not found OpenSSL ; if you installed it, you may need to set -DNEKO_NETWORK_LIBRARY_PATH=<path>")
endif()

# On Windows, if CURL uses Schannel, we don't need OpenSSL
if(WIN32 AND CURL_USES_SCHANNEL)
    message(STATUS "CURL is using Schannel on Windows, OpenSSL is optional")
    set(NEKO_NETWORK_USE_OPENSSL FALSE)
else()
    set(NEKO_NETWORK_USE_OPENSSL ${OPENSSL_FOUND})
endif()

# Create CURL::libcurl alias target if it doesn't exist
if (NOT TARGET CURL::libcurl)

    if (TARGET CURL::libcurl_static)
        add_library(CURL::libcurl ALIAS CURL::libcurl_static)
    endif()

    if (TARGET CURL::libcurl_shared)
        add_library(CURL::libcurl ALIAS CURL::libcurl_shared)
    endif()

endif()

# Create CURL::libcurl target if it doesn't exist (for older FindCURL.cmake)
if(CURL_FOUND AND NOT TARGET CURL::libcurl)
    add_library(CURL::libcurl UNKNOWN IMPORTED)
    set_target_properties(CURL::libcurl PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${CURL_LIBRARIES}"
    )
    if(CURL_LIBRARY)
        set_target_properties(CURL::libcurl PROPERTIES
            IMPORTED_LOCATION "${CURL_LIBRARY}"
        )
    endif()
endif()



if(NEKO_NETWORK_AUTO_FETCH_DEPS)
    include(FetchContent)

    if(NOT NekoSchema_FOUND)
        message(STATUS "NekoSchema not found; Neko Network is Fetching NekoSchema...")
        FetchContent_Declare(
            NekoSchema
            GIT_REPOSITORY https://github.com/moehoshio/NekoSchema.git
            GIT_TAG        main
        )
        set(NEKO_SCHEMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(NekoSchema)
    endif()

    if(NOT NekoFunction_FOUND)
        message(STATUS "NekoFunction not found; Neko Network is Fetching NekoFunction...")
        FetchContent_Declare(
            NekoFunction
            GIT_REPOSITORY https://github.com/moehoshio/NekoFunction.git
            GIT_TAG        main
        )
        set(NEKO_FUNCTION_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(NEKO_FUNCTION_STATIC_LINK ${NEKO_NETWORK_STATIC_LINK} CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(NekoFunction)
    endif()

    if(NOT NekoSystem_FOUND)
        message(STATUS "NekoSystem not found; Neko Network is Fetching NekoSystem...")
        FetchContent_Declare(
            NekoSystem
            GIT_REPOSITORY https://github.com/moehoshio/NekoSystem
            GIT_TAG        main
        )
        set(NEKO_SYSTEM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(NekoSystem)
    endif()

    # Optionally
    if(NOT NekoLog_FOUND)
        message(STATUS "NekoLog not found; Neko Network is Fetching NekoLog...")
        FetchContent_Declare(
            NekoLog
            GIT_REPOSITORY https://github.com/moehoshio/NekoLog.git
            GIT_TAG        main
        )
        set(NEKO_LOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(NekoLog)
    endif()

    if(NOT GTest_FOUND AND NEKO_NETWORK_BUILD_TESTS)
        message(STATUS "GTest not found; Neko Network is Fetching GoogleTest...")
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.17.0
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

endif()

# ================
# = Main Target =
# ================

add_library(NekoNetwork STATIC src/neko/network/network.cpp)
add_library(Neko::Network ALIAS NekoNetwork)

target_include_directories(NekoNetwork PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Dependencies
target_link_libraries(NekoNetwork PUBLIC
    # Neko
    NekoSchema NekoLog NekoFunction NekoSystem
    # libcurl
    CURL::libcurl
)
target_compile_features(NekoNetwork PUBLIC cxx_std_20)

if(WIN32)
    target_link_libraries(NekoNetwork PUBLIC ws2_32) # Windows Sockets
    
    # If using Schannel, link Windows SSL libraries
    if(CURL_USES_SCHANNEL)
        target_link_libraries(NekoNetwork PUBLIC Crypt32)
        message(STATUS "Linking Crypt32 for Schannel SSL support")
    endif()
endif()

# Link OpenSSL if needed (not on Windows with Schannel)
if(NEKO_NETWORK_USE_OPENSSL)
    target_link_libraries(NekoNetwork PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    message(STATUS "Linking OpenSSL libraries")

    # On Windows with OpenSSL static linking, additional system libraries are required
    if(WIN32 AND NEKO_NETWORK_STATIC_LINK)
        target_link_libraries(NekoNetwork PUBLIC
            Crypt32
            advapi32
            user32
            bcrypt
        )
        message(STATUS "Linking Windows system libraries for OpenSSL static build")
    endif()
endif()

# ================
# ===== Test =====
# ================

if(NEKO_NETWORK_BUILD_TESTS)
    enable_testing()
    message(STATUS "NekoNetwork tests enabled (NEKO_NETWORK_BUILD_TESTS=ON)")

    if (NOT GTEST_FOUND AND NOT NEKO_NETWORK_AUTO_FETCH_DEPS)
        message(WARNING "GTest is required for building tests but was not found.")
        message(FATAL_ERROR "Please enable -DNEKO_NETWORK_AUTO_FETCH_DEPS=ON or install GTest and make it discoverable by CMake. use -DNEKO_NETWORK_LIBRARY_PATH=</path/to/gtest>")
    endif()

    add_executable(NekoNetwork_test tests/network_test.cpp)
    target_include_directories(NekoNetwork_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(NekoNetwork_test PRIVATE NekoNetwork GTest::gtest GTest::gtest_main)
    target_compile_features(NekoNetwork_test PRIVATE cxx_std_20)

    include(NekoRunTimeCopy)
    NekoRunTimeCopy(NekoNetwork_test)

    include(GoogleTest)
    gtest_discover_tests(NekoNetwork_test 
        DISCOVERY_MODE PRE_TEST
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
else()
    message(STATUS "NekoNetwork tests disabled (NEKO_NETWORK_BUILD_TESTS=OFF)")
endif()

# ================
# == Install =====
# ================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING 
    PATTERN "*.hpp"
    PATTERN "*.cppm"
)

# Install targets
install(TARGETS NekoNetwork
    EXPORT NekoNetworkTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install export targets
install(EXPORT NekoNetworkTargets
    FILE NekoNetworkTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NekoNetwork
)

# Create and install Config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/NekoNetworkConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/NekoNetworkConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NekoNetwork
)

# Create and install Version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/NekoNetworkConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/NekoNetworkConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NekoNetworkConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NekoNetwork
)

message(STATUS "Neko Network End of configuration")
