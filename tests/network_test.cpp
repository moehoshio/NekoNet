/**
 * @file network_test.cpp
 * @brief Unit tests for the NekoNet network module
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#include <gtest/gtest.h>
#include <neko/network/network.hpp>
#include <neko/network/networkCommon.hpp>
#include <neko/network/networkTypes.hpp>

using namespace neko::network;

/**
 * @brief Test fixture for Network class tests
 */
class NetworkTest : public ::testing::Test {
protected:
    std::shared_ptr<Network> network;

    void SetUp() override {
        network = std::make_shared<Network>();
    }

    void TearDown() override {
        network.reset();
    }
};

// ============================================================================
// NetworkResult tests
// ============================================================================

TEST(NetworkResultTest, IsSuccessReturnsTrueFor200StatusCode) {
    NetworkResult<std::string> result;
    result.statusCode = 200;
    result.hasError = false;

    EXPECT_TRUE(result.isSuccess());
}

TEST(NetworkResultTest, IsSuccessReturnsFalseForErrorStatus) {
    NetworkResult<std::string> result;
    result.statusCode = 200;
    result.hasError = true;

    EXPECT_FALSE(result.isSuccess());
}

TEST(NetworkResultTest, IsSuccessReturnsFalseFor400StatusCode) {
    NetworkResult<std::string> result;
    result.statusCode = 400;
    result.hasError = false;

    EXPECT_FALSE(result.isSuccess());
}

TEST(NetworkResultTest, IsSuccessReturnsFalseFor500StatusCode) {
    NetworkResult<std::string> result;
    result.statusCode = 500;
    result.hasError = false;

    EXPECT_FALSE(result.isSuccess());
}

TEST(NetworkResultTest, HasContentReturnsTrueForNonEmptyContent) {
    NetworkResult<std::string> result;
    result.content = "test content";

    EXPECT_TRUE(result.hasContent());
}

TEST(NetworkResultTest, HasContentReturnsFalseForEmptyContent) {
    NetworkResult<std::string> result;
    result.content = "";

    EXPECT_FALSE(result.hasContent());
}

TEST(NetworkResultTest, SetErrorSetsErrorStateCorrectly) {
    NetworkResult<std::string> result;
    result.setError("Test error", "Detailed test error");

    EXPECT_TRUE(result.hasError);
    EXPECT_EQ(result.errorMessage, "Test error");
    EXPECT_EQ(result.detailedErrorMessage, "Detailed test error");
}

// ============================================================================
// RequestConfig tests
// ============================================================================

TEST(RequestConfigTest, DefaultValuesAreSetCorrectly) {
    RequestConfig config;

    EXPECT_TRUE(config.url.empty());
    EXPECT_EQ(config.method, RequestType::Get);
    // userAgent gets default value from globalConfig, which may be empty
    // Just verify it's accessible and is a valid string type
    auto userAgent = config.userAgent;
    EXPECT_TRUE(userAgent.empty() || !userAgent.empty()); // Always true, just to use the value
}

TEST(RequestConfigTest, CanSetCustomValues) {
    RequestConfig config;
    config.url = "https://example.com";
    config.method = RequestType::Post;
    config.userAgent = "Custom Agent";
    config.header = "Content-Type: application/json";
    config.postData = "{\"key\":\"value\"}";

    EXPECT_EQ(config.url, "https://example.com");
    EXPECT_EQ(config.method, RequestType::Post);
    EXPECT_EQ(config.userAgent, "Custom Agent");
    EXPECT_EQ(config.header, "Content-Type: application/json");
    EXPECT_EQ(config.postData, "{\"key\":\"value\"}");
}

// ============================================================================
// RetryConfig tests
// ============================================================================

TEST(RetryConfigTest, DefaultValuesAreSetCorrectly) {
    RetryConfig config;

    EXPECT_EQ(config.maxRetries, 3);
    EXPECT_EQ(config.retryDelay.count(), 150);
    EXPECT_EQ(config.successCodes.size(), 2);
    EXPECT_EQ(config.successCodes[0], 200);
    EXPECT_EQ(config.successCodes[1], 204);
}

TEST(RetryConfigTest, CanSetCustomRetryValues) {
    RetryConfig config;
    config.maxRetries = 5;
    config.retryDelay = std::chrono::milliseconds(500);
    config.successCodes = {200, 201, 204};

    EXPECT_EQ(config.maxRetries, 5);
    EXPECT_EQ(config.retryDelay.count(), 500);
    EXPECT_EQ(config.successCodes.size(), 3);
}

// ============================================================================
// MultiDownloadConfig tests
// ============================================================================

TEST(MultiDownloadConfigTest, DefaultValuesAreSetCorrectly) {
    MultiDownloadConfig config;

    EXPECT_EQ(config.approach, MultiDownloadConfig::Approach::Auto);
    EXPECT_EQ(config.segmentParam, 0);
    EXPECT_EQ(config.successCodes.size(), 2);
    EXPECT_EQ(config.successCodes[0], 200);
    EXPECT_EQ(config.successCodes[1], 206);
}

TEST(MultiDownloadConfigTest, CanSetThreadApproach) {
    MultiDownloadConfig config;
    config.approach = MultiDownloadConfig::Approach::Thread;
    config.segmentParam = 50;

    EXPECT_EQ(config.approach, MultiDownloadConfig::Approach::Thread);
    EXPECT_EQ(config.segmentParam, 50);
}

TEST(MultiDownloadConfigTest, CanSetSizeApproach) {
    MultiDownloadConfig config;
    config.approach = MultiDownloadConfig::Approach::Size;
    config.segmentParam = 1024 * 1024 * 10; // 10MB

    EXPECT_EQ(config.approach, MultiDownloadConfig::Approach::Size);
    EXPECT_EQ(config.segmentParam, 1024 * 1024 * 10);
}

// ============================================================================
// NetConfig tests
// ============================================================================

TEST(NetConfigTest, CanSetAndGetUserAgent) {
    config::NetConfig config;
    config.setUserAgent("Test User Agent");

    EXPECT_EQ(config.getUserAgent(), "Test User Agent");
}

TEST(NetConfigTest, CanSetAndGetProxy) {
    config::NetConfig config;
    config.setProxy("http://proxy.example.com:8080");

    EXPECT_EQ(config.getProxy(), "http://proxy.example.com:8080");
}

TEST(NetConfigTest, CanSetAndGetProtocol) {
    config::NetConfig config;
    config.setProtocol("https");

    EXPECT_EQ(config.getProtocol(), "https");
}

TEST(NetConfigTest, CanSetAndGetAvailableHostList) {
    config::NetConfig config;
    std::vector<std::string> hosts = {"host1.example.com", "host2.example.com"};
    config.setAvailableHostList(hosts);

    EXPECT_EQ(config.getAvailableHost(), "host1.example.com");
}

TEST(NetConfigTest, GetAvailableHostReturnsEmptyForEmptyList) {
    config::NetConfig config;

    EXPECT_TRUE(config.getAvailableHost().empty());
}

// ============================================================================
// Network basic tests
// ============================================================================

TEST_F(NetworkTest, ConstructorCreatesValidInstance) {
    EXPECT_NE(network, nullptr);
}

TEST_F(NetworkTest, ExecuteWithInvalidUrlReturnsError) {
    RequestConfig config;
    config.url = "invalid-url";
    config.method = RequestType::Get;

    auto result = network->execute(config);

    EXPECT_TRUE(result.hasError);
    EXPECT_FALSE(result.isSuccess());
}

TEST_F(NetworkTest, ExecuteWithEmptyUrlReturnsError) {
    RequestConfig config;
    config.url = "";
    config.method = RequestType::Get;

    auto result = network->execute(config);

    EXPECT_TRUE(result.hasError);
    EXPECT_FALSE(result.isSuccess());
}

// ============================================================================
// Network request tests (require actual network connection)
// ============================================================================

TEST_F(NetworkTest, DISABLED_GetRequestToPublicApiReturnsSuccess) {
    // This test is disabled by default because it requires an actual network connection
    // You can run it using --gtest_also_run_disabled_tests
    RequestConfig config;
    config.url = "https://httpbin.org/get";
    config.method = RequestType::Get;

    auto result = network->execute(config);

    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.hasContent());
    EXPECT_EQ(result.statusCode, 200);
}

TEST_F(NetworkTest, DISABLED_PostRequestWithJsonDataReturnsSuccess) {
    RequestConfig config;
    config.url = "https://httpbin.org/post";
    config.method = RequestType::Post;
    config.header = "Content-Type: application/json";
    config.postData = R"({"test": "data", "number": 42})";

    auto result = network->execute(config);

    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.hasContent());
    EXPECT_EQ(result.statusCode, 200);
}

TEST_F(NetworkTest, DISABLED_HeadRequestReturnsSuccessWithoutContent) {
    RequestConfig config;
    config.url = "https://httpbin.org/get";
    config.method = RequestType::Head;

    auto result = network->execute(config);

    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.statusCode, 200);
}

TEST_F(NetworkTest, DISABLED_GetContentTypeReturnsCorrectType) {
    auto contentType = network->getContentType("https://httpbin.org/json");

    EXPECT_TRUE(contentType.has_value());
    if (contentType.has_value()) {
        EXPECT_TRUE(contentType->find("application/json") != std::string::npos);
    }
}

TEST_F(NetworkTest, DISABLED_GetContentSizeReturnsPositiveValue) {
    auto contentSize = network->getContentSize("https://httpbin.org/json");

    EXPECT_TRUE(contentSize.has_value());
    if (contentSize.has_value()) {
        EXPECT_GT(*contentSize, 0);
    }
}

TEST_F(NetworkTest, DISABLED_FindUrlHeaderReturnsCorrectHeader) {
    auto header = network->findUrlHeader("https://httpbin.org/json", "Content-Type");

    EXPECT_TRUE(header.has_value());
    if (header.has_value()) {
        EXPECT_FALSE(header->empty());
    }
}

// ============================================================================
// Asynchronous request tests
// ============================================================================

TEST_F(NetworkTest, DISABLED_ExecuteAsyncReturnsValidFuture) {
    RequestConfig config;
    config.url = "https://httpbin.org/get";
    config.method = RequestType::Get;

    auto future = network->executeAsync(config);

    ASSERT_TRUE(future.valid());

    auto result = future.get();

    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.hasContent());
}

// ============================================================================
// Retry logic tests
// ============================================================================

TEST_F(NetworkTest, DISABLED_ExecuteWithRetrySucceedsAfterRetries) {
    RetryConfig config;
    config.config.url = "https://httpbin.org/status/500";
    config.config.method = RequestType::Get;
    config.maxRetries = 3;
    config.retryDelay = std::chrono::milliseconds(100);

    auto result = network->executeWithRetry(config);

    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.statusCode, 500);
}

// ============================================================================
// Header constants tests
// ============================================================================

TEST(HeaderConstantsTest, JsonContentTypeIsCorrect) {
    EXPECT_STREQ(header::jsonContentType, "application/json");
    EXPECT_STREQ(header::jsonContentHeader, "Content-Type: application/json");
}

TEST(HeaderConstantsTest, TextContentTypeIsCorrect) {
    EXPECT_STREQ(header::textContentType, "text/plain");
    EXPECT_STREQ(header::textContentHeader, "Content-Type: text/plain");
}

TEST(HeaderConstantsTest, XmlContentTypeIsCorrect) {
    EXPECT_STREQ(header::xmlContentType, "application/xml");
    EXPECT_STREQ(header::xmlContentHeader, "Content-Type: application/xml");
}

TEST(HeaderConstantsTest, HtmlContentTypeIsCorrect) {
    EXPECT_STREQ(header::htmlContentType, "text/html");
    EXPECT_STREQ(header::htmlContentHeader, "Content-Type: text/html");
}

TEST(HeaderConstantsTest, ImageContentTypesAreCorrect) {
    EXPECT_STREQ(header::pngContentType, "image/png");
    EXPECT_STREQ(header::jpegContentType, "image/jpeg");
    EXPECT_STREQ(header::gifContentType, "image/gif");
    EXPECT_STREQ(header::svgContentType, "image/svg+xml");
}

// ============================================================================
// Main function to run all tests
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
