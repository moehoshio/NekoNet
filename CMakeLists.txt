cmake_minimum_required(VERSION 3.16)
project(NekoNet VERSION 1.0 LANGUAGES CXX)

# ================
# ==== Config ====
# ================
option(NEKO_NET_AUTO_FETCH_DEPS "Neko Net Automatically fetch dependencies" ON)
option(NEKO_NET_BUILD_TESTS "Neko Net Build tests" ON)

set(NEKO_NET_LIBRARY_PATH "" CACHE PATH "Path to look for dependencies (OpenSSL, libcurl, GTest)")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (DEFINED NEKO_NET_LIBRARY_PATH)
    find_package(OpenSSL
        HINTS "${NEKO_NET_LIBRARY_PATH}"
    )
    find_package(CURL REQUIRED
        HINTS "${NEKO_NET_LIBRARY_PATH}"
    )
    find_package(GTest CONFIG 
        HINTS "${NEKO_NET_LIBRARY_PATH}"
        QUIET
    )
else()
    find_package(OpenSSL)
    find_package(CURL REQUIRED)
    find_package(GTest CONFIG QUIET)
endif()

# Print configuration summary
message(STATUS "Start configuration Neko Net...")
message(STATUS "")
message(STATUS "NekoNet configuration summary:")
message(STATUS "")
message(STATUS "  - CMake version: ${CMAKE_VERSION}")
message(STATUS "")
message(STATUS "  - Neko Net Auto fetch deps: ${NEKO_NET_AUTO_FETCH_DEPS}")
message(STATUS "  - Neko Net Build tests: ${NEKO_NET_BUILD_TESTS}")
message(STATUS "")
message(STATUS "Dependency summary:")
message(STATUS "  - OpenSSL support: ${OPENSSL_FOUND} version: ${OPENSSL_VERSION}")
message(STATUS "  - libcurl support: ${CURL_FOUND} version: ${CURL_VERSION_STRING}")
message(STATUS "  - GTest : ${GTest_FOUND} version : ${GTest_VERSION}")
message(STATUS "")

# Check SSL backend requirements
if (NOT CURL_FOUND)
    message(FATAL_ERROR "libcurl not found ; if you installed it, you may need to set CURL_ROOT_DIR or -DCMAKE_PREFIX_PATH=<path>")
endif()

# On Windows, OpenSSL is optional if using Schannel (native SSL)
# On other platforms, OpenSSL is required
if (NOT WIN32 AND NOT OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found ; if you installed it, you may need to set OPENSSL_ROOT_DIR or -DCMAKE_PREFIX_PATH=<path>")
endif()

if (OPENSSL_FOUND)
    message(STATUS "Found OpenSSL ${OPENSSL_VERSION}")
elseif(WIN32)
    message(STATUS "OpenSSL not found - using Windows Schannel (native SSL)")
endif()

message(STATUS "Found libcurl ${CURL_VERSION_STRING}")

if(NEKO_NET_AUTO_FETCH_DEPS)

    include(FetchContent)

    FetchContent_Declare(
        NekoSchema
        GIT_REPOSITORY https://github.com/moehoshio/NekoSchema.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(NekoSchema)

    FetchContent_Declare(
        NekoFunction
        GIT_REPOSITORY https://github.com/moehoshio/NekoFunction.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(NekoFunction)

    FetchContent_Declare(
        NekoSystem
        GIT_REPOSITORY https://github.com/moehoshio/NekoSystem
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(NekoSystem)

    # Optionally
    FetchContent_Declare(
        NekoLog
        GIT_REPOSITORY https://github.com/moehoshio/NekoLog.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(NekoLog)

    if(NOT GTEST_FOUND AND NEKO_NET_BUILD_TESTS)
        message(STATUS "GTest not found; Neko Net Fetching GoogleTest...")

        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        release-1.12.1
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

endif()

# ================
# = Main Target =
# ================

add_library(NekoNet STATIC src/neko/network/network.cpp)
add_library(Neko::Net ALIAS NekoNet)

target_include_directories(NekoNet PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Dependencies
target_link_libraries(NekoNet PUBLIC
    # Neko
    NekoSchema NekoLog NekoFunction NekoSystem
    # libcurl
    CURL::libcurl
)

# Link OpenSSL if found (optional on Windows when using Schannel)
if(OPENSSL_FOUND)
    target_link_libraries(NekoNet PUBLIC OpenSSL::SSL OpenSSL::Crypto)

    # On Windows with OpenSSL static linking, additional system libraries are required
    if(WIN32 AND VCPKG_TARGET_TRIPLET MATCHES "static")
        target_link_libraries(NekoNet PUBLIC
            Crypt32
            ws2_32
            advapi32
            user32
            bcrypt
        )
        message(STATUS "Linking Windows system libraries for OpenSSL static build")
    endif()
else()
    # Using native SSL (Schannel on Windows)
    if(WIN32)
        target_link_libraries(NekoNet PUBLIC
            ws2_32  # Windows Sockets
        )
        message(STATUS "Using Windows Schannel - minimal system libraries linked")
    endif()
endif()

target_compile_features(NekoNet PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(NekoNet PUBLIC /Zc:__cplusplus)
endif()

# ================
# ===== Test =====
# ================

if(NEKO_NET_BUILD_TESTS)
    enable_testing()
    message(STATUS "NekoNet tests enabled (NEKO_NET_BUILD_TESTS=ON)")

    if (NOT GTEST_FOUND AND NOT NEKO_NET_AUTO_FETCH_DEPS)
        message(STATUS "GTest is required for building tests but was not found.")
        message(FATAL_ERROR "Please enable -DNEKO_NET_AUTO_FETCH_DEPS=ON or install GTest and make it discoverable by CMake. e.g -DCMAKE_PREFIX_PATH=</path/to/googletest>")
    endif()

    add_executable(NekoNet_tests tests/network_test.cpp)
    target_include_directories(NekoNet_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(NekoNet_tests PRIVATE NekoNet GTest::gtest GTest::gtest_main)
    target_compile_features(NekoNet_tests PRIVATE cxx_std_20)

    # Automatically copy runtime libraries (DLLs on Windows, .so on Linux, .dylib on macOS)
    include(NekoRunTimeCopy)
    NekoRunTimeCopy(NekoNet_tests)

    include(GoogleTest)
    # Use DISCOVERY_MODE PRE_TEST to avoid running tests at build time
    # This prevents DLL loading issues during the build process
    gtest_discover_tests(NekoNet_tests 
        DISCOVERY_MODE PRE_TEST
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else()
    message(STATUS "NekoNet tests disabled (NEKO_NET_BUILD_TESTS=OFF)")
endif()

message(STATUS "Neko Net End of configuration")
