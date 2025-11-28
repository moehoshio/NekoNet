cmake_minimum_required(VERSION 3.15)
project(vcpkg_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(NekoNetwork REQUIRED)

add_executable(vcpkg_test vcpkg_test.cpp)

target_link_libraries(vcpkg_test PRIVATE Neko::Network)