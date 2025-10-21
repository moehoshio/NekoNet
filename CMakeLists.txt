cmake_minimum_required(VERSION 3.16)
project(NekoNet VERSION 1.0 LANGUAGES CXX)

# ================
# ==== Config ====
# ================

option(NEKO_NET_AUTO_FETCH_DEPS "Neko Net Automatically fetch dependencies" ON)
option(NEKO_NET_BUILD_TESTS "Neko Net Build tests" ON)
option(NEKO_NET_STATIC_LINK "Neko Net Static Link library" OFF)

set(NEKO_NET_LIBRARY_PATH "" CACHE PATH "Path to look for dependencies (OpenSSL, libcurl, GTest)")

if(NEKO_NET_LIBRARY_PATH)
    list(APPEND CMAKE_PREFIX_PATH ${NEKO_NET_LIBRARY_PATH})
endif(NEKO_NET_LIBRARY_PATH)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(OpenSSL QUIET)
find_package(CURL REQUIRED)
find_package(GTest QUIET)


# Print configuration summary
message(STATUS "Start configuration Neko Net...")
message(STATUS "")
message(STATUS "NekoNet configuration summary:")
message(STATUS "")
message(STATUS "  - CMake version: ${CMAKE_VERSION}")
message(STATUS "  - Neko Net Library Path: ${NEKO_NET_LIBRARY_PATH}")
message(STATUS "  - CMake Prefix Path: ${CMAKE_PREFIX_PATH}")
message(STATUS "")
message(STATUS "  - Neko Net Auto fetch deps: ${NEKO_NET_AUTO_FETCH_DEPS}")
message(STATUS "  - Neko Net Build tests: ${NEKO_NET_BUILD_TESTS}")
message(STATUS "  - Neko Net Static Link library: ${NEKO_NET_STATIC_LINK}")
message(STATUS "")
message(STATUS "Dependency summary:")
message(STATUS "  - OpenSSL support: ${OPENSSL_FOUND} version: ${OPENSSL_VERSION}")
message(STATUS "  - libcurl support: ${CURL_FOUND} version: ${CURL_VERSION_STRING}")
message(STATUS "  - GTest : ${GTest_FOUND} version : ${GTest_VERSION}")
message(STATUS "")

if (NOT CURL_FOUND)
    message(FATAL_ERROR "Neko Net not found libcurl ; if you installed it, you may need to set -DNEKO_NET_LIBRARY_PATH=<path> or -DCMAKE_PREFIX_PATH=<path>")
endif()

# On Windows, OpenSSL is optional if using Schannel (native SSL)
# On other platforms, OpenSSL is required
if (NOT WIN32 AND NOT OPENSSL_FOUND)
    message(FATAL_ERROR "Neko Net not found OpenSSL ; if you installed it, you may need to set -DNEKO_NET_LIBRARY_PATH=<path> or -DCMAKE_PREFIX_PATH=<path>")
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
    set(NEKO_FUNCTION_LIBRARY_PATH ${CMAKE_PREFIX_PATH})
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
        message(STATUS "GTest not found; Neko Net is Fetching GoogleTest...")

        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.17.0
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
target_compile_features(NekoNet PUBLIC cxx_std_20)

if(WIN32)
    target_link_libraries(NekoNet PUBLIC ws2_32) # Windows Sockets
endif()

if(MSVC)
    target_compile_options(NekoNet PUBLIC /Zc:__cplusplus)
endif()

# Link OpenSSL if found (optional on Windows when using Schannel)
if(OPENSSL_FOUND)
    target_link_libraries(NekoNet PUBLIC OpenSSL::SSL OpenSSL::Crypto)

    # On Windows with OpenSSL static linking, additional system libraries are required
    if(WIN32 AND NEKO_NET_STATIC_LINK)
        target_link_libraries(NekoNet PUBLIC
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

if(NEKO_NET_BUILD_TESTS)
    enable_testing()
    message(STATUS "NekoNet tests enabled (NEKO_NET_BUILD_TESTS=ON)")

    if (NOT GTEST_FOUND AND NOT NEKO_NET_AUTO_FETCH_DEPS)
        message(WARNING "GTest is required for building tests but was not found.")
        message(FATAL_ERROR "Please enable -DNEKO_NET_AUTO_FETCH_DEPS=ON or install GTest and make it discoverable by CMake. use -DNEKO_NET_LIBRARY_PATH=</path/to/gtest>")
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
