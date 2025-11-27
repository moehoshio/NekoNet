# Neko Network

Neko Network is a modern, easy-to-use, and efficient C++20 network library built on top of libcurl.

[![License](https://img.shields.io/badge/License-MIT%20OR%20Apache--2.0-blue.svg)](LICENSE)
![Require](https://img.shields.io/badge/%20Require%20-%3E=%20C++%2020-orange.svg)
[![CMake](https://img.shields.io/badge/CMake-3.16+-green.svg)](https://cmake.org/)
[![CI Status](https://github.com/moehoshio/NekoNetwork/actions/workflows/ci.yml/badge.svg)](https://github.com/moehoshio/NekoNetwork/actions/workflows/ci.yml)

## Features

- **Modern C++20** - Built with modern C++ features and concepts
- **Easy to Use** - Simple and intuitive API
- **Template-Based** - Generic response types (string, binary, custom types)
- **Multiple Request Types** - GET, POST, HEAD, Download, Upload
- **Asynchronous Support** - Execute requests asynchronously with futures
- **Retry Mechanism** - Built-in retry logic for failed requests
- **Multi-threaded Downloads** - Split large files into segments for faster downloads
- **Progress Tracking** - Callback support for monitoring download/upload progress
- **Flexible Configuration** - Extensive configuration options for all request types
- **Resumable Downloads** - Support for resuming interrupted downloads
- **Custom Headers** - Easy custom header management
- **Proxy Support** - Built-in proxy configuration
- **Extensible Logging** - Custom logger support with NekoLog integration
- **Custom Executors** - Replace default async executor with thread pools
- **Global Configuration** - Centralized configuration for default settings

## Requirements

- C++20 or higher compatible compiler
- CMake 3.16 or higher
- Git
- libcurl (with OpenSSL support)
- OpenSSL

## Platform Support

Neko Network is tested on the following platforms:

- **Windows** - MSVC 2022+ (Debug/Release)
- **Linux** - GCC 10+, Clang 12+ (Debug/Release)
- **macOS** - Apple Clang 13+ (Debug/Release)

All platforms support:

- Multi-threaded operations
- Asynchronous execution
- Resumable downloads
- Proxy configuration

## Dependencies

Neko Network depends on the following libraries:

### Required

- **libcurl** (8.0+) - HTTP client library with SSL support
- **OpenSSL** (3.0+) - SSL/TLS encryption

### Optional (Auto-fetched via CMake)

- **NekoSchema** - Type definitions
- **NekoFunction** - Utility functions
- **NekoSystem** - System utilities
- **NekoLog** - Logging framework
- **GoogleTest** - Testing framework (only for tests)

## Quick Start

Configure:
[CMake](#cmake) | [Static Linking](#static-linking) | [Tests](#testing)

Example:
[Basic](#basic-example) | [GET Request](#get-request) | [POST Request](#post-request) | [Download File](#download-file) | [Async Request](#asynchronous-requests)

Advanced:
[Custom Response Types](#custom-response-types) | [Retry Logic](#retry-logic) | [Multi-threaded Download](#multi-threaded-download) | [Custom Headers](#custom-headers) | [Progress Callback](#progress-callback) | [Custom Logger](#custom-logger) | [Custom Executor](#custom-async-executor) | [Global Config](#global-configuration)

### CMake

1. Using CMake's `FetchContent` to include Neko Network in your project:

```cmake
include(FetchContent)

# Add Neko Network to your CMake project
FetchContent_Declare(
    NekoNetwork
    GIT_REPOSITORY https://github.com/moehoshio/NekoNetwork.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(NekoNetwork)

# Add your target and link NekoNetwork
add_executable(your_target main.cpp)

target_link_libraries(your_target PRIVATE Neko::Network)
```

2. Include the header in your source code

```cpp
#include <neko/network/network.hpp>
```

3. Let CMake find libcurl and OpenSSL (if you have already installed the dependencies)

```bash
cmake -B ./build . -DNEKO_NETWORK_LIBRARY_PATH="/path/to/" -S .
```

### Static Linking

If you want to use static linking, make sure all libraries (OpenSSL, libcurl..) are built as static libraries. (Neko Network itself is always a static library)
Enable the static linking option in your CMake configuration:

```bash
cmake -B ./build . -DNEKO_NETWORK_STATIC_LINK=ON -DNEKO_NETWORK_LIBRARY_PATH="/path/to/x64-windows-static" -S .
```

### Basic Example

Now you can start making network requests with minimal setup:

```cpp
#include <neko/network/network.hpp>

int main() {
    using namespace neko::network;
    
    // Create a network instance
    Network network;
    
    // Configure and execute a simple GET request
    RequestConfig config;
    config.url = "https://api.example.com/data";
    
    auto result = network.execute(config);
    
    if (result.isSuccess()) {
        std::cout << "Response: " << result.content << std::endl;
    } else {
        std::cerr << "Error: " << result.errorMessage << std::endl;
    }
    
    return 0;
}
```

## Usage

### GET Request

The simplest way to make a GET request:

```cpp
using namespace neko::network;

Network network;

RequestConfig config;
config.url = "https://api.github.com/users/moehoshio";
config.method = RequestType::Get;

auto result = network.execute(config);

if (result.isSuccess()) {
    std::cout << "Status: " << result.statusCode << std::endl;
    std::cout << "Content: " << result.content << std::endl;
}
```

### POST Request

Sending data with a POST request:

```cpp
using namespace neko::network;

Network network;

RequestConfig config;
config.url = "https://api.example.com/submit";
config.method = RequestType::Post;
config.postData = R"({"name": "John", "age": 30})";
config.header = "Content-Type: application/json";

auto result = network.execute(config);

if (result.isSuccess()) {
    std::cout << "Response: " << result.content << std::endl;
}
```

### Download File

Download a file to disk:

```cpp
using namespace neko::network;

Network network;

RequestConfig config;
config.url = "https://example.com/largefile.zip";
config.method = RequestType::DownloadFile;
config.fileName = "downloaded_file.zip";

auto result = network.execute(config);

if (result.isSuccess()) {
    std::cout << "File downloaded successfully!" << std::endl;
}
```

#### Resumable Download

Resume an interrupted download:

```cpp
RequestConfig config;
config.url = "https://example.com/largefile.zip";
config.method = RequestType::DownloadFile;
config.fileName = "downloaded_file.zip";
config.resumable = true;  // Enable resume support

auto result = network.execute(config);
```

This assumes the existing contents of the file are correct and resumes downloading the remaining portion from where the transfer was interrupted.

### Asynchronous Requests

Execute requests asynchronously without blocking:

```cpp
using namespace neko::network;

Network network;

RequestConfig config;
config.url = "https://api.example.com/data";

// Execute asynchronously
auto future = network.executeAsync(config);

// Do other work here...
std::cout << "Request in progress..." << std::endl;

// Wait for the result
auto result = future.get();

if (result.isSuccess()) {
    std::cout << "Async response: " << result.content << std::endl;
}
```

### Retry Logic

Automatically retry failed requests with configurable settings:

```cpp
using namespace neko::network;

Network network;

RetryConfig retryConfig;
retryConfig.config.url = "https://api.example.com/unstable";
retryConfig.maxRetries = 5;  // Try up to 5 times
retryConfig.retryDelay = std::chrono::milliseconds(500);  // Wait 500ms between retries
retryConfig.successCodes = {200, 201, 204};  // Consider these as success

auto result = network.executeWithRetry(retryConfig);

if (result.isSuccess()) {
    std::cout << "Request succeeded (possibly after retries)" << std::endl;
}
```

### Multi-threaded Download

Download large files faster by splitting them into segments:

```cpp
using namespace neko::network;

Network network;

MultiDownloadConfig multiConfig;
multiConfig.config.url = "https://example.com/largefile.iso";
multiConfig.config.fileName = "largefile.iso";

// Auto mode: automatically determine the best approach
multiConfig.approach = MultiDownloadConfig::Auto;

// Or specify the number of threads
// multiConfig.approach = MultiDownloadConfig::Thread;
// multiConfig.segmentParam = 8;  // Use 8 threads

// Or specify segment size
// multiConfig.approach = MultiDownloadConfig::Size;
// multiConfig.segmentParam = 5 * 1024 * 1024;  // 5MB per segment

bool success = network.multiThreadedDownload(multiConfig);

if (success) {
    std::cout << "Multi-threaded download completed!" << std::endl;
}
```

#### Download Approaches

- **Auto**: Automatically determines the best approach based on file size
- **Thread**: Split by number of threads (default: 100 tasks)
- **Size**: Split by segment size in bytes (default: 5MB)
- **Quantity**: Split by number of segments (default: 100 segments)

### Custom Headers

Add custom headers to your requests:

```cpp
using namespace neko::network;

Network network;

RequestConfig config;
config.url = "https://api.example.com/protected";
config.header = "Authorization: Bearer your_token_here\n"
                "Accept: application/json\n"
                "X-Custom-Header: custom_value";

auto result = network.execute(config);
```

### Progress Callback

Monitor download/upload progress in real-time:

```cpp
using namespace neko::network;

Network network;

RequestConfig config;
config.url = "https://example.com/largefile.zip";
config.method = RequestType::DownloadFile;
config.fileName = "largefile.zip";

// Set up progress callback
config.progressCallback = [](neko::uint64 bytesReceived) {
    std::cout << "Downloaded: " << bytesReceived << " bytes\r" << std::flush;
};

auto result = network.execute(config);
```

### Proxy Support

Configure proxy settings for your requests:

```cpp
using namespace neko::network;

Network network;

RequestConfig config;
config.url = "https://api.example.com/data";
config.proxy = "http://proxy.example.com:8080";  // Specify proxy
// config.proxy = "true";  // Use system proxy
// config.proxy = "";  // No proxy (default)

auto result = network.execute(config);
```

### User Agent

Customize the User-Agent header:

```cpp
using namespace neko::network;

Network network;

RequestConfig config;
config.url = "https://api.example.com/data";
config.userAgent = "MyApp/1.0 (Custom User Agent)";

auto result = network.execute(config);
```

### Utility Functions

#### Get Content Type

Retrieve the content type of a resource:

```cpp
using namespace neko::network;

Network network;

auto contentType = network.getContentType("https://example.com/image.png");

if (contentType) {
    std::cout << "Content-Type: " << *contentType << std::endl;
    // Example output: "image/png"
}
```

#### Get Content Size

Get the size of a resource before downloading:

```cpp
using namespace neko::network;

Network network;

auto size = network.getContentSize("https://example.com/largefile.zip");

if (size) {
    std::cout << "File size: " << *size << " bytes" << std::endl;
}
```

#### Find Custom Header

Retrieve any header value from a resource:

```cpp
using namespace neko::network;

Network network;

auto lastModified = network.findUrlHeader("https://example.com/data.json", "Last-Modified");

if (lastModified) {
    std::cout << "Last-Modified: " << *lastModified << std::endl;
}
```

## Advanced Features

### Custom Response Types

**All execution methods are template functions**, allowing you to specify custom return types for the response content. This enables direct deserialization into your preferred data structure.

#### Supported Types

By default, NekoNetwork provides explicit template instantiations for:

- `std::string` - Text responses (default)
- `std::vector<char>` - Binary data
- `std::fstream` - Direct file writing (for download operations)

#### Using Different Response Types

```cpp
using namespace neko::network;

Network network;

// 1. String response (default)
RequestConfig config;
config.url = "https://api.example.com/text";
auto textResult = network.execute<std::string>(config);
// textResult.content is std::string

// 2. Binary response
config.url = "https://api.example.com/binary";
auto binaryResult = network.execute<std::vector<char>>(config);
// binaryResult.content is std::vector<char>

// 3. Async with custom type
config.url = "https://api.example.com/data";
auto future = network.executeAsync<std::vector<char>>(config);
auto result = future.get();
// result.content is std::vector<char>

// 4. Retry with custom type
RetryConfig retryConfig;
retryConfig.config.url = "https://api.example.com/data";
auto retryResult = network.executeWithRetry<std::vector<char>>(retryConfig);
// retryResult.content is std::vector<char>
```

#### Custom Type Requirements

To use your own custom type `T` as the response type:

1. **Implement a `writeToCallback` specialization** for your type:

```cpp
namespace neko::network::helper {
    template <>
    inline neko::uint64 writeToCallback<MyCustomType>(
        char *ptr, neko::uint64 size, neko::uint64 nmemb, void *userdata
    ) {
        auto *ctx = static_cast<WriteCallbackContext<MyCustomType> *>(userdata);
        neko::uint64 written = size * nmemb;
        
        // Your custom logic to append data to ctx->buffer
        ctx->buffer->appendData(ptr, written);
        
        ctx->totalBytes += written;
        if (ctx->progressCallback && *ctx->progressCallback) {
            (*ctx->progressCallback)(ctx->totalBytes);
        }
        return written;
    }
}
```

2. **Add explicit template instantiation** in your implementation file:

```cpp
// In your .cpp file
#include <neko/network/network.hpp>

namespace neko::network {
    // Explicit template instantiations for MyCustomType
    template NetworkResult<MyCustomType> Network::execute(const RequestConfig &);
    template std::future<NetworkResult<MyCustomType>> Network::executeAsync(const RequestConfig &);
    template NetworkResult<MyCustomType> Network::executeWithRetry(const RetryConfig &);
}
```

#### Example: JSON Deserialization

```cpp
#include <neko/network/network.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace neko::network;

// Specialize for json type
namespace neko::network::helper {
    template <>
    inline neko::uint64 writeToCallback<json>(
        char *ptr, neko::uint64 size, neko::uint64 nmemb, void *userdata
    ) {
        auto *ctx = static_cast<WriteCallbackContext<json> *>(userdata);
        neko::uint64 written = size * nmemb;
        
        // Accumulate JSON string
        std::string data(ptr, written);
        
        // Note: For actual use, you'd want to accumulate in a string buffer first
        // and parse at the end, but this demonstrates the concept
        
        ctx->totalBytes += written;
        if (ctx->progressCallback && *ctx->progressCallback) {
            (*ctx->progressCallback)(ctx->totalBytes);
        }
        return written;
    }
}

// In your .cpp file - add explicit instantiation
namespace neko::network {
    template NetworkResult<json> Network::execute(const RequestConfig &);
    template std::future<NetworkResult<json>> Network::executeAsync(const RequestConfig &);
}

// Usage
int main() {
    Network network;
    
    RequestConfig config;
    config.url = "https://api.example.com/users.json";
    
    auto result = network.execute<json>(config);
    
    if (result.isSuccess()) {
        // result.content is already parsed JSON
        std::string name = result.content["name"];
        int age = result.content["age"];
        std::cout << "User: " << name << ", Age: " << age << std::endl;
    }
    
    return 0;
}
```

#### Example: Protocol Buffer

```cpp
#include <neko/network/network.hpp>
#include "user.pb.h"  // Your protobuf generated header

using namespace neko::network;

// Specialize for protobuf type
namespace neko::network::helper {
    template <>
    inline neko::uint64 writeToCallback<UserProto>(
        char *ptr, neko::uint64 size, neko::uint64 nmemb, void *userdata
    ) {
        auto *ctx = static_cast<WriteCallbackContext<UserProto> *>(userdata);
        neko::uint64 written = size * nmemb;
        
        // Parse protobuf data
        if (!ctx->buffer->ParseFromArray(ptr, static_cast<int>(written))) {
            return 0;  // Indicate error
        }
        
        ctx->totalBytes += written;
        if (ctx->progressCallback && *ctx->progressCallback) {
            (*ctx->progressCallback)(ctx->totalBytes);
        }
        return written;
    }
}

// Usage
Network network;
RequestConfig config;
config.url = "https://api.example.com/user.pb";

auto result = network.execute<UserProto>(config);

if (result.isSuccess()) {
    // result.content is parsed protobuf object
    std::cout << "User: " << result.content.name() << std::endl;
}
```

### Request Configuration

The `RequestConfig` structure provides extensive configuration options:

```cpp
struct RequestConfig {
    std::string url;                    // Target URL
    RequestType method;                 // GET, POST, HEAD, DownloadFile, UploadFile
    std::string userAgent;              // Custom User-Agent
    std::string proxy;                  // Proxy configuration
    std::string requestId;              // Unique identifier for the request
    std::string header;                 // Custom headers (newline-separated)
    std::string postData;               // POST request body
    std::string fileName;               // File path for upload/download
    bool resumable;                     // Enable resumable downloads
    std::string range;                  // Byte range for partial downloads
    std::function<void(uint64)> progressCallback;  // Progress tracking
};
```

### Error Handling

The `NetworkResult` structure provides comprehensive error information:

```cpp
template <typename T>
struct NetworkResult {
    int statusCode;                 // HTTP status code
    bool hasError;                  // Error flag
    T content;                      // Response content
    std::string errorMessage;       // Brief error message
    std::string detailedErrorMessage;  // Detailed error information
    
    bool isSuccess() const;         // Check if request succeeded
    bool hasContent() const;        // Check if response has content
};
```

Example error handling:

```cpp
auto result = network.execute(config);

if (!result.isSuccess()) {
    std::cerr << "Error occurred!" << std::endl;
    std::cerr << "Status Code: " << result.statusCode << std::endl;
    std::cerr << "Error: " << result.errorMessage << std::endl;
    
    if (!result.detailedErrorMessage.empty()) {
        std::cerr << "Details: " << result.detailedErrorMessage << std::endl;
    }
}
```

### Custom Logger

NekoNetwork uses an internal logging system that can be customized to integrate with your application's logging infrastructure.

By default, NekoNetwork automatically uses:
- **NLogLogger** if NekoLog is available
- **DefaultLogger** (stdout/stderr) as fallback

#### Using Custom Logger

Implement the `ILogger` interface and register your custom logger factory:

```cpp
using namespace neko::network;

// Step 1: Create your custom logger class
class MyCustomLogger : public log::ILogger {
public:
    void error(const std::string &msg) override {
        // Your custom error logging
        myLogSystem->logError("NekoNetwork", msg);
    }
    
    void info(const std::string &msg) override {
        // Your custom info logging
        myLogSystem->logInfo("NekoNetwork", msg);
    }
    
    void warn(const std::string &msg) override {
        // Your custom warning logging
        myLogSystem->logWarning("NekoNetwork", msg);
    }
    
    void debug(const std::string &msg) override {
        // Your custom debug logging
        myLogSystem->logDebug("NekoNetwork", msg);
    }

private:
    MyLogSystem* myLogSystem = MyLogSystem::getInstance();
};

// Step 2: Register the custom logger factory globally
int main() {
    // Set the logger factory before creating any Network instances
    log::setLoggerFactory([]() -> std::shared_ptr<log::ILogger> {
        return std::make_shared<MyCustomLogger>();
    });
    
    // Now all Network instances will use your custom logger
    Network network;
    network.execute(config);  // Will log using MyCustomLogger
    
    return 0;
}
```

#### Per-Instance Logger

You can also provide a custom logger to individual Network instances:

```cpp
// Create a custom logger instance
auto customLogger = std::make_shared<MyCustomLogger>();

// Pass it to the Network constructor
Network network(
    executor::createExecutor(),  // Use default executor
    customLogger                  // Use custom logger
);
```

### Custom Async Executor

NekoNetwork uses an executor system for asynchronous operations. You can replace the default `std::async` executor with your own implementation (e.g., thread pool).

By default, NekoNetwork uses `StdAsyncExecutor` which wraps `std::async`.

#### Using Custom Executor

Implement the `IAsyncExecutor` interface and register your custom executor factory:

```cpp
using namespace neko::network;

// Step 1: Create your custom executor class
class ThreadPoolExecutor : public executor::IAsyncExecutor {
public:
    ThreadPoolExecutor(size_t numThreads) : pool(numThreads) {}
    
protected:
    template <typename R>
    std::future<R> submitImpl(std::function<R()> f) override {
        // Submit to your thread pool instead of std::async
        return pool.submit(std::move(f));
    }

private:
    MyThreadPool pool;
};

// Step 2: Register the custom executor factory globally
int main() {
    // Set the executor factory before creating any Network instances
    executor::setExecutorFactory([]() -> std::shared_ptr<executor::IAsyncExecutor> {
        return std::make_shared<ThreadPoolExecutor>(8);  // 8 threads
    });
    
    // Now all Network instances will use your thread pool
    Network network;
    auto future = network.executeAsync(config);  // Uses ThreadPoolExecutor
    
    return 0;
}
```

#### Per-Instance Executor

You can also provide a custom executor to individual Network instances:

```cpp
// Create a custom executor instance
auto customExecutor = std::make_shared<ThreadPoolExecutor>(16);

// Pass it to the Network constructor
Network network(
    customExecutor,              // Use custom executor
    log::createLogger()          // Use default logger
);

// Now async operations use your thread pool
auto future = network.executeAsync(config);
```

#### Example: Integration with NekoThreadPool

If you're using NekoThreadPool, here's a complete integration example:

```cpp
#include <neko/network/network.hpp>
#include <neko/threadpool/thread_pool.hpp>

using namespace neko::network;

class NekoThreadPoolExecutor : public executor::IAsyncExecutor {
public:
    NekoThreadPoolExecutor(std::shared_ptr<neko::ThreadPool> pool) 
        : threadPool(pool) {}
    
protected:
    template <typename R>
    std::future<R> submitImpl(std::function<R()> f) override {
        // Submit to NekoThreadPool
        return threadPool->submit(std::move(f));
    }

private:
    std::shared_ptr<neko::ThreadPool> threadPool;
};

int main() {
    // Create a thread pool
    auto pool = std::make_shared<neko::ThreadPool>(8);
    
    // Create Network with custom executor
    auto customExecutor = std::make_shared<NekoThreadPoolExecutor>(pool);
    Network network(customExecutor, log::createLogger());
    
    // Execute multiple async requests
    std::vector<std::future<NetworkResult<>>> futures;
    
    for (const auto& url : urls) {
        RequestConfig config;
        config.url = url;
        futures.push_back(network.executeAsync(config));
    }
    
    // Wait for all results
    for (auto& future : futures) {
        auto result = future.get();
        // Process result...
    }
    
    return 0;
}
```

### Global Configuration

NekoNetwork provides a global configuration system for setting default values:

```cpp
using namespace neko::network;

// Initialize with custom configuration
initialize([](config::NetConfig& cfg) {
    cfg.setUserAgent("MyApp/2.0")
       .setProtocol("https://")
       .setProxy("http://proxy.example.com:8080")
       .setAvailableHostList({"api.example.com", "api-backup.example.com"});
});

// Or use default configuration
initialize(nullptr);

// Now RequestConfig will use these defaults automatically
RequestConfig config;
config.url = "https://api.example.com/data";
// config.userAgent is automatically set to "MyApp/2.0"
// config.proxy is automatically set to "http://proxy.example.com:8080"
```

#### Global Configuration Options

```cpp
config::NetConfig& globalConfig = config::globalConfig;

// Set User-Agent for all requests
globalConfig.setUserAgent("MyApp/1.0");

// Set proxy for all requests
globalConfig.setProxy("http://proxy.example.com:8080");

// Set protocol (http:// or https://)
globalConfig.setProtocol("https://");

// Set available hosts for failover
globalConfig.setAvailableHostList({"api1.example.com", "api2.example.com"});

// Add a host to the list
globalConfig.pushAvailableHost("api3.example.com");

// Clear all hosts
globalConfig.clearAvailableHost();

// Clear all configuration
globalConfig.clear();
```

#### Building URLs with Global Config

```cpp
using namespace neko::network;

// Configure global settings
initialize([](config::NetConfig& cfg) {
    cfg.setProtocol("https://")
       .setAvailableHostList({"api.example.com"});
});

// Build URL using global config
std::string url = buildUrl("/users/123");
// Result: "https://api.example.com/users/123"

// Or specify custom host and protocol
std::string customUrl = buildUrl("/data", "custom.example.com", "http://");
// Result: "http://custom.example.com/data"
```

## Testing

You can run the tests to verify that everything is working correctly.

If you haven't configured the build yet, please run:

```shell
cmake -B ./build . -DNEKO_NETWORK_BUILD_TESTS=ON -DNEKO_NETWORK_AUTO_FETCH_DEPS=ON -S .
```

Now, you can build the test files (you must build them manually at least once before running the tests!).

```shell
cmake --build ./build --config Debug
```

Then, you can run the tests with the following commands:

```shell
cd ./build && ctest --output-on-failure
```

If everything is set up correctly, you should see output similar to the following:

```shell
Test project /path/to/NekoNetwork/build
    Start  1: NetworkResultTest.IsSuccessReturnsTrueFor200StatusCode
1/27 Test  #1: NetworkResultTest.IsSuccessReturnsTrueFor200StatusCode ...   Passed    0.00 sec

    # ......

    Start 27: HeaderConstantsTest.AcceptEncodingHeader
27/27 Test #27: HeaderConstantsTest.AcceptEncodingHeader .................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 27

Total Test time (real) =   0.15 sec
```

### Disable Tests

If you want to disable building and running tests, you can set the following CMake option when configuring your project:

```shell
cmake -B ./build . -DNEKO_NETWORK_BUILD_TESTS=OFF -S .
```

This will skip test targets during the build process.

## Documentation

For more detailed information, please refer to:

- **[Advanced Usage Guide](docs/ADVANCED_USAGE.md)** - Comprehensive guide for custom loggers, executors, and global configuration
- **[API Reference](include/neko/network/)** - Full API documentation in header files
- **[Examples](tests/network_test.cpp)** - Unit tests serve as usage examples

## License

[License](LICENSE) MIT OR Apache-2.0

## See More

- [NekoLog](https://github.com/moehoshio/NekoLog): An easy-to-use, modern, lightweight, and efficient C++20 logging library.
- [NekoEvent](https://github.com/moehoshio/NekoEvent): A modern easy to use type-safe and high-performance event handling system for C++.
- [NekoSchema](https://github.com/moehoshio/NekoSchema): A lightweight, header-only C++20 schema library.
- [NekoSystem](https://github.com/moehoshio/NekoSystem): A modern C++20 cross-platform system utility library.
- [NekoNetwork](https://github.com/moehoshio/NekoNetwork): A modern , easy-to-use C++20 networking library via libcurl.
- [NekoFunction](https://github.com/moehoshio/NekoFunction): A comprehensive modern C++ utility library that provides practical functions for common programming tasks.
- [NekoThreadPool](https://github.com/moehoshio/NekoThreadPool): An easy to use and efficient C++ 20 thread pool that supports priorities and submission to specific threads.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Support

If you encounter any issues or have questions, please file an issue on the [GitHub issue tracker](https://github.com/moehoshio/NekoNetwork/issues).
