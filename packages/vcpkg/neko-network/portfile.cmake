vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO moehoshio/NekoNetwork
    REF v1.0.2
    SHA512 b7925a81671ca0b7a8107ccd4aa1ac4c51f2c02d8e0aed482b0e89bcd4c75edad140c8417a3cfafaa61ef664741d259772de913198062299773210d8079f0cdf
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DNEKO_NETWORK_BUILD_TESTS=OFF
        -DNEKO_NETWORK_AUTO_FETCH_DEPS=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/NekoNetwork PACKAGE_NAME nekonetwork)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")


