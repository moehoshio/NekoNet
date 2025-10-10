cmake_minimum_required(VERSION 3.16)
project(NekoNet VERSION 1.0 LANGUAGES CXX)

option(NEKO_AUTO_FETCH_DEPS "Automatically fetch dependencies" ON)
option(NEKO_BUILD_TESTS "Build tests" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(OpenSSL)
find_package(CURL)
find_package(GTest QUIET)

# Print configuration summary
message(STATUS "NekoFunction configuration summary:")
message(STATUS "  - CMake version: ${CMAKE_VERSION}")
message(STATUS "")
message(STATUS "  - Neko Auto fetch deps: ${NEKO_AUTO_FETCH_DEPS}")
message(STATUS "  - Neko Build tests: ${NEKO_BUILD_TESTS}")
message(STATUS "")
message(STATUS "Dependency summary:")
message(STATUS "  - GoogleTest support: ${GTest_FOUND}")
message(STATUS "  - OpenSSL support: ${OpenSSL_FOUND}")
message(STATUS "  - libcurl support: ${CURL_FOUND}")
message(STATUS "")


if (NOT OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found ; if you installed it, you may need to set OPENSSL_ROOT_DIR or -DCMAKE_PREFIX_PATH=<path>")
endif()

if (NOT CURL_FOUND)
    message(FATAL_ERROR "libcurl not found ; if you installed it, you may need to set CURL_ROOT_DIR or -DCMAKE_PREFIX_PATH=<path>")
endif()

message(STATUS "Found OpenSSL ${OPENSSL_VERSION}")
message(STATUS "Found libcurl ${CURL_VERSION_STRING}")

if(NEKO_AUTO_FETCH_DEPS)

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

    if(NOT GTest_FOUND AND NEKO_BUILD_TESTS)
        message(STATUS "GTest not found. Fetching GoogleTest...")

        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        release-1.12.1
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

endif(NEKO_AUTO_FETCH_DEPS)


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
    # OpenSSL
    OpenSSL::SSL OpenSSL::Crypto
    # libcurl
    CURL::libcurl
)

target_compile_features(NekoNet PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(NekoNet PUBLIC /Zc:__cplusplus)
endif()


if(NEKO_BUILD_TESTS)
    enable_testing()
    message(STATUS "NekoNet tests enabled")

    if (NOT GTest_FOUND AND NOT NEKO_AUTO_FETCH_DEPS)
        message(FATAL_ERROR "GTest not found. Please install GoogleTest or enable NEKO_AUTO_FETCH_DEPS to automatically fetch it.")
    endif()

    add_executable(NekoNet_tests tests/network_test.cpp)
    target_include_directories(NekoNet_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(NekoNet_tests PRIVATE NekoNet gtest gtest_main)
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
    message(STATUS "NekoNet tests disabled (NEKO_BUILD_TESTS=OFF)")
endif()
