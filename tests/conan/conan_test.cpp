/**
 * @file conan_test.cpp
 * @brief Minimal integration test for NekoNetwork via Conan
 */

#include <neko/network/network.hpp>
#include <neko/network/networkCommon.hpp>
#include <iostream>

int main() {
    std::cout << "=== NekoNetwork Conan Integration Test ===" << std::endl;

    try {
        // Initialize network configuration
        neko::network::initialize([](neko::network::config::NetConfig& cfg) {
            cfg.setProtocol("https://")
               .setUserAgent("NekoNetwork-conan-test/1.0");
        });

        // Create a Network instance
        auto network = std::make_shared<neko::network::Network>();

        // Test basic GET request
        neko::network::RequestConfig config;
        config.url = "https://httpbin.org/get";
        config.method = neko::network::RequestType::Get;

        std::cout << "Testing GET request to: " << config.url << std::endl;
        
        auto result = network->execute(config);

        if (result.isSuccess()) {
            std::cout << "✓ Request successful!" << std::endl;
            std::cout << "  Status code: " << result.statusCode << std::endl;
            std::cout << "  Content length: " << result.content.length() << " bytes" << std::endl;
        } else {
            std::cout << "✗ Request failed!" << std::endl;
            std::cout << "  Error: " << result.errorMessage << std::endl;
            return 1;
        }

        std::cout << "\n=== Conan Integration Test Passed ===" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
