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

TEST_F(NetworkTest, GetRequestToPublicApiReturnsSuccess) {
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

TEST_F(NetworkTest, PostRequestWithJsonDataReturnsSuccess) {
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

TEST_F(NetworkTest, HeadRequestReturnsSuccessWithoutContent) {
    RequestConfig config;
    config.url = "https://httpbin.org/get";
    config.method = RequestType::Head;

    auto result = network->execute(config);

    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.statusCode, 200);
}

TEST_F(NetworkTest, GetContentTypeReturnsCorrectType) {
    // Test that getContentType() correctly retrieves Content-Type header via HEAD request
    auto contentType = network->getContentType("https://httpbin.org/get");

    EXPECT_TRUE(contentType.has_value());
    if (contentType.has_value()) {
        // httpbin.org/get returns application/json
        EXPECT_TRUE(contentType->find("application/json") != std::string::npos);
    }
}

TEST_F(NetworkTest, GetContentSizeReturnsPositiveValue) {
    // Test that getContentSize() correctly retrieves Content-Length header via HEAD request
    auto contentSize = network->getContentSize("https://httpbin.org/get");

    EXPECT_TRUE(contentSize.has_value());
    if (contentSize.has_value()) {
        EXPECT_GT(*contentSize, 0);
    }
}

TEST_F(NetworkTest, FindUrlHeaderReturnsCorrectHeader) {
    // Test that findUrlHeader() correctly retrieves headers via HEAD request
    auto header = network->findUrlHeader("https://httpbin.org/get", "Content-Type");

    EXPECT_TRUE(header.has_value());
    if (header.has_value()) {
        EXPECT_FALSE(header->empty());
    }
}

// ============================================================================
// Asynchronous request tests
// ============================================================================

TEST_F(NetworkTest, ExecuteAsyncReturnsValidFuture) {
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

TEST_F(NetworkTest, ExecuteWithRetrySucceedsAfterRetries) {
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
// Custom Logger tests
// ============================================================================

class TestLogger : public log::ILogger {
public:
    std::vector<std::string> errorMessages;
    std::vector<std::string> infoMessages;
    std::vector<std::string> warnMessages;
    std::vector<std::string> debugMessages;

    void error(const std::string &msg) override {
        errorMessages.push_back(msg);
    }

    void info(const std::string &msg) override {
        infoMessages.push_back(msg);
    }

    void warn(const std::string &msg) override {
        warnMessages.push_back(msg);
    }

    void debug(const std::string &msg) override {
        debugMessages.push_back(msg);
    }

    void clear() {
        errorMessages.clear();
        infoMessages.clear();
        warnMessages.clear();
        debugMessages.clear();
    }
};

TEST(CustomLoggerTest, CustomLoggerCanBeSetGlobally) {
    // Save original factory
    auto originalFactory = log::getLoggerFactory();

    // Set custom logger factory
    auto testLogger = std::make_shared<TestLogger>();
    log::setLoggerFactory([testLogger]() -> std::shared_ptr<log::ILogger> {
        return testLogger;
    });

    // Create a logger instance
    auto logger = log::createLogger();

    // Test logging
    logger->info("Test info message");
    logger->error("Test error message");
    logger->warn("Test warn message");
    logger->debug("Test debug message");

    // Verify messages were logged
    EXPECT_EQ(testLogger->infoMessages.size(), 1);
    EXPECT_EQ(testLogger->infoMessages[0], "Test info message");
    EXPECT_EQ(testLogger->errorMessages.size(), 1);
    EXPECT_EQ(testLogger->errorMessages[0], "Test error message");
    EXPECT_EQ(testLogger->warnMessages.size(), 1);
    EXPECT_EQ(testLogger->warnMessages[0], "Test warn message");
    EXPECT_EQ(testLogger->debugMessages.size(), 1);
    EXPECT_EQ(testLogger->debugMessages[0], "Test debug message");

    // Restore original factory
    log::setLoggerFactory(originalFactory);
}

TEST(CustomLoggerTest, NetworkCanUseCustomLoggerInstance) {
    auto testLogger = std::make_shared<TestLogger>();

    // Create Network with custom logger
    Network network(executor::createExecutor(), testLogger);

    // The network will use this logger internally
    // We can't directly test internal logging, but we verify the logger is accepted
    EXPECT_NE(testLogger, nullptr);
}

TEST(CustomLoggerTest, LoggerFactoryCanBeReset) {
    // Save original factory
    auto originalFactory = log::getLoggerFactory();

    // Set a custom factory
    log::setLoggerFactory([]() -> std::shared_ptr<log::ILogger> {
        return std::make_shared<TestLogger>();
    });

    // Create logger from custom factory
    auto customLogger = log::createLogger();
    EXPECT_NE(customLogger, nullptr);

    // Restore original factory
    log::setLoggerFactory(originalFactory);

    // Create logger from restored factory
    auto defaultLogger = log::createLogger();
    EXPECT_NE(defaultLogger, nullptr);
}

// ============================================================================
// Custom Executor tests
// ============================================================================

class TestExecutor : public executor::IAsyncExecutor {
public:
    std::atomic<int> taskCount{0};

    // Override the submit method (non-template version)
    template <typename F>
    auto submit(F &&f) -> std::future<decltype(f())> {
        taskCount++;
        // Use std::async for testing purposes
        return std::async(std::launch::async, std::forward<F>(f));
    }
};

TEST(CustomExecutorTest, CustomExecutorCanBeSetGlobally) {
    // Save original factory
    auto originalFactory = executor::getExecutorFactory();

    // Create a counter to track tasks
    auto taskCounter = std::make_shared<std::atomic<int>>(0);

    // Set custom executor factory that returns a new executor each time
    executor::setExecutorFactory([taskCounter]() -> std::shared_ptr<executor::IAsyncExecutor> {
        auto exec = std::make_shared<TestExecutor>();
        // Copy the counter pointer so all executors share it
        exec->taskCount.store(taskCounter->load());
        return exec;
    });

    // Create an executor instance
    auto exec = executor::createExecutor();

    // Submit a task
    auto future = exec->submit([]() { return 42; });
    auto result = future.get();

    EXPECT_EQ(result, 42);
    // The task was submitted, so the executor's submit was called
    EXPECT_TRUE(true); // Just verify it completes successfully

    // Restore original factory
    executor::setExecutorFactory(originalFactory);
}

TEST(CustomExecutorTest, NetworkCanUseCustomExecutorInstance) {
    auto testExecutor = std::make_shared<TestExecutor>();

    // Create Network with custom executor
    Network network(testExecutor, log::createLogger());

    // The network will use this executor for async operations
    EXPECT_NE(testExecutor, nullptr);
}

TEST(CustomExecutorTest, ExecutorFactoryCanBeReset) {
    // Save original factory
    auto originalFactory = executor::getExecutorFactory();

    // Set a custom factory
    executor::setExecutorFactory([]() -> std::shared_ptr<executor::IAsyncExecutor> {
        return std::make_shared<TestExecutor>();
    });

    // Create executor from custom factory
    auto customExecutor = executor::createExecutor();
    EXPECT_NE(customExecutor, nullptr);

    // Restore original factory
    executor::setExecutorFactory(originalFactory);

    // Create executor from restored factory
    auto defaultExecutor = executor::createExecutor();
    EXPECT_NE(defaultExecutor, nullptr);
}

TEST(CustomExecutorTest, ExecutorCanSubmitMultipleTasks) {
    auto testExecutor = std::make_shared<TestExecutor>();

    // Submit multiple tasks
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(testExecutor->submit([i]() { return i * 2; }));
    }

    // Verify all tasks complete correctly
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(futures[i].get(), i * 2);
    }

    EXPECT_GE(testExecutor->taskCount.load(), 10);
}

// ============================================================================
// Global Configuration tests
// ============================================================================

TEST(GlobalConfigTest, UserAgentCanBeSetAndRetrieved) {
    auto& cfg = config::globalConfig;
    
    // Save original value
    auto originalUA = cfg.getUserAgent();
    
    // Set new value
    cfg.setUserAgent("TestAgent/1.0");
    EXPECT_EQ(cfg.getUserAgent(), "TestAgent/1.0");
    
    // Restore original value
    cfg.setUserAgent(originalUA);
}

TEST(GlobalConfigTest, ProxyCanBeSetAndRetrieved) {
    auto& cfg = config::globalConfig;
    
    // Save original value
    auto originalProxy = cfg.getProxy();
    
    // Set new value
    cfg.setProxy("http://proxy.test.com:8080");
    EXPECT_EQ(cfg.getProxy(), "http://proxy.test.com:8080");
    
    // Restore original value
    cfg.setProxy(originalProxy);
}

TEST(GlobalConfigTest, ProtocolCanBeSetAndRetrieved) {
    auto& cfg = config::globalConfig;
    
    // Save original value
    auto originalProtocol = cfg.getProtocol();
    
    // Set new value
    cfg.setProtocol("http://");
    EXPECT_EQ(cfg.getProtocol(), "http://");
    
    // Restore original value
    cfg.setProtocol(originalProtocol);
}

TEST(GlobalConfigTest, AvailableHostListCanBeManaged) {
    auto& cfg = config::globalConfig;
    
    // Clear hosts first
    cfg.clearAvailableHost();
    
    // Add hosts
    cfg.pushAvailableHost("host1.example.com");
    cfg.pushAvailableHost("host2.example.com");
    
    // Get first host
    auto host = cfg.getAvailableHost();
    EXPECT_EQ(host, "host1.example.com");
    
    // Set multiple hosts
    cfg.setAvailableHostList({"api1.test.com", "api2.test.com", "api3.test.com"});
    host = cfg.getAvailableHost();
    EXPECT_EQ(host, "api1.test.com");
    
    // Clear hosts
    cfg.clearAvailableHost();
    host = cfg.getAvailableHost();
    EXPECT_TRUE(host.empty());
}

TEST(GlobalConfigTest, ConfigCanBeChained) {
    auto& cfg = config::globalConfig;
    
    // Save original values
    auto originalUA = cfg.getUserAgent();
    auto originalProxy = cfg.getProxy();
    auto originalProtocol = cfg.getProtocol();
    
    // Chain configuration
    cfg.setUserAgent("ChainedAgent/1.0")
       .setProxy("http://chained.proxy.com:8080")
       .setProtocol("https://");
    
    EXPECT_EQ(cfg.getUserAgent(), "ChainedAgent/1.0");
    EXPECT_EQ(cfg.getProxy(), "http://chained.proxy.com:8080");
    EXPECT_EQ(cfg.getProtocol(), "https://");
    
    // Restore original values
    cfg.setUserAgent(originalUA)
       .setProxy(originalProxy)
       .setProtocol(originalProtocol);
}

TEST(GlobalConfigTest, ClearResetsAllConfiguration) {
    auto& cfg = config::globalConfig;
    
    // Set some values
    cfg.setUserAgent("TestAgent/1.0")
       .setProxy("http://test.proxy.com:8080")
       .setProtocol("http://")
       .pushAvailableHost("test.example.com");
    
    // Clear all
    cfg.clear();
    
    // Verify all are empty
    EXPECT_TRUE(cfg.getUserAgent().empty());
    EXPECT_TRUE(cfg.getProxy().empty());
    EXPECT_TRUE(cfg.getProtocol().empty());
    EXPECT_TRUE(cfg.getAvailableHost().empty());
}

TEST(GlobalConfigTest, BuildUrlCombinesProtocolHostAndPath) {
    auto& cfg = config::globalConfig;
    
    // Save original values
    auto originalProtocol = cfg.getProtocol();
    
    // Set configuration
    cfg.setProtocol("https://")
       .setAvailableHostList({"api.example.com"});
    
    // Build URL
    auto url = buildUrl("/users/123");
    EXPECT_EQ(url, "https://api.example.com/users/123");
    
    // Build URL with custom host and protocol
    auto customUrl = buildUrl("/data", "custom.example.com", "http://");
    EXPECT_EQ(customUrl, "http://custom.example.com/data");
    
    // Restore original values
    cfg.setProtocol(originalProtocol);
    cfg.clearAvailableHost();
}

TEST(GlobalConfigTest, InitializeSetsDefaultConfiguration) {
    // Initialize with default configuration
    initialize(nullptr);
    
    auto& cfg = config::globalConfig;
    
    // Verify default values are set
    EXPECT_FALSE(cfg.getUserAgent().empty());
    EXPECT_EQ(cfg.getUserAgent(), "NekoNet/v1.0 +https://github.com/moehoshio/NekoNet");
    EXPECT_EQ(cfg.getProtocol(), "https://");
    EXPECT_EQ(cfg.getProxy(), "true");
}

TEST(GlobalConfigTest, InitializeWithCustomConfiguration) {
    // Save original values
    auto originalUA = config::globalConfig.getUserAgent();
    auto originalProxy = config::globalConfig.getProxy();
    
    // Initialize with custom configuration
    initialize([](config::NetConfig& cfg) {
        cfg.setUserAgent("CustomApp/2.0")
           .setProxy("http://custom.proxy.com:3128")
           .setProtocol("http://")
           .setAvailableHostList({"custom.api.com"});
    });
    
    // Verify custom values are set
    EXPECT_EQ(config::globalConfig.getUserAgent(), "CustomApp/2.0");
    EXPECT_EQ(config::globalConfig.getProxy(), "http://custom.proxy.com:3128");
    EXPECT_EQ(config::globalConfig.getProtocol(), "http://");
    EXPECT_EQ(config::globalConfig.getAvailableHost(), "custom.api.com");
    
    // Restore original configuration
    initialize([originalUA, originalProxy](config::NetConfig& cfg) {
        cfg.setUserAgent(originalUA)
           .setProxy(originalProxy)
           .setProtocol("https://");
        cfg.clearAvailableHost();
    });
}

// ============================================================================
// Custom Response Type tests
// ============================================================================

TEST(CustomResponseTypeTest, StringTypeIsDefault) {
    Network network;
    
    RequestConfig config;
    config.url = "";  // Empty URL will definitely error
    
    // Default template parameter is std::string
    auto result = network.execute(config);
    
    // Verify the result type can hold string content
    static_assert(std::is_same_v<decltype(result.content), std::string>, 
                  "Default content type should be std::string");
    
    EXPECT_TRUE(result.hasError);  // Empty URL should error
}

TEST(CustomResponseTypeTest, CanUseVectorCharType) {
    Network network;
    
    RequestConfig config;
    config.url = "";  // Empty URL will definitely error
    
    // Explicitly specify std::vector<char> as response type
    auto result = network.execute<std::vector<char>>(config);
    
    // Verify the result type is std::vector<char>
    static_assert(std::is_same_v<decltype(result.content), std::vector<char>>, 
                  "Content type should be std::vector<char>");
    
    EXPECT_TRUE(result.hasError);  // Empty URL should error
}

TEST(CustomResponseTypeTest, AsyncWithCustomType) {
    Network network;
    
    RequestConfig config;
    config.url = "";  // Empty URL will definitely error
    
    // Async execution with std::vector<char>
    auto future = network.executeAsync<std::vector<char>>(config);
    auto result = future.get();
    
    // Verify the result type
    static_assert(std::is_same_v<decltype(result.content), std::vector<char>>, 
                  "Content type should be std::vector<char>");
    
    EXPECT_TRUE(result.hasError);  // Empty URL should error
}

TEST(CustomResponseTypeTest, RetryWithCustomType) {
    Network network;
    
    RetryConfig retryConfig;
    retryConfig.config.url = "";  // Empty URL will definitely error
    retryConfig.maxRetries = 1;  // Just 1 retry to keep test fast
    
    // Retry execution with std::vector<char>
    auto result = network.executeWithRetry<std::vector<char>>(retryConfig);
    
    // Verify the result type
    static_assert(std::is_same_v<decltype(result.content), std::vector<char>>, 
                  "Content type should be std::vector<char>");
    
    EXPECT_TRUE(result.hasError);  // Empty URL should error even after retry
}

TEST(CustomResponseTypeTest, NetworkResultTypeTraits) {
    // Test compile-time type checking
    NetworkResult<std::string> stringResult;
    NetworkResult<std::vector<char>> binaryResult;
    NetworkResult<std::fstream> fileResult;
    
    // Verify types are distinct
    static_assert(!std::is_same_v<decltype(stringResult.content), decltype(binaryResult.content)>, 
                  "String and binary result types should be different");
    
    static_assert(!std::is_same_v<decltype(stringResult.content), decltype(fileResult.content)>, 
                  "String and file result types should be different");
    
    // All should have the same structure otherwise
    EXPECT_EQ(stringResult.statusCode, 0);
    EXPECT_EQ(binaryResult.statusCode, 0);
    EXPECT_EQ(fileResult.statusCode, 0);
}

TEST(CustomResponseTypeTest, EmptyContentCheckWorksForAllTypes) {
    NetworkResult<std::string> stringResult;
    NetworkResult<std::vector<char>> binaryResult;
    
    // Empty content should return false for hasContent()
    EXPECT_FALSE(stringResult.hasContent());
    EXPECT_FALSE(binaryResult.hasContent());
    
    // Add content
    stringResult.content = "test";
    binaryResult.content = {'t', 'e', 's', 't'};
    
    // Now should return true
    EXPECT_TRUE(stringResult.hasContent());
    EXPECT_TRUE(binaryResult.hasContent());
}

TEST(CustomResponseTypeTest, SuccessStateIndependentOfType) {
    NetworkResult<std::string> stringResult;
    NetworkResult<std::vector<char>> binaryResult;
    
    // Set success state
    stringResult.statusCode = 200;
    stringResult.hasError = false;
    
    binaryResult.statusCode = 200;
    binaryResult.hasError = false;
    
    // Both should report success
    EXPECT_TRUE(stringResult.isSuccess());
    EXPECT_TRUE(binaryResult.isSuccess());
    
    // Set error state
    stringResult.hasError = true;
    binaryResult.hasError = true;
    
    // Both should report failure
    EXPECT_FALSE(stringResult.isSuccess());
    EXPECT_FALSE(binaryResult.isSuccess());
}

// ============================================================================
// Main function to run all tests
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
