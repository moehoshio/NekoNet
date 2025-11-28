cmake_minimum_required(VERSION 3.15)
project(conan_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(NekoNetwork REQUIRED)

add_executable(conan_test conan_test.cpp)

target_link_libraries(conan_test PRIVATE Neko::Network)