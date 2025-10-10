# NekoNet Documentation

Welcome to the NekoNet documentation! This page provides an overview of all available documentation resources.

## 📚 Documentation Structure

### Getting Started

- **[README.md](../README.md)** - Main documentation with quick start guide, basic usage, and examples
  - Installation (CMake, Manual)
  - Basic examples (GET, POST, Download, Async)
  - Common use cases
  - Testing guide
  - Platform support

### Advanced Topics

- **[ADVANCED_USAGE.md](ADVANCED_USAGE.md)** - Comprehensive guide for advanced features
  - Custom Logger implementation and integration
  - Custom Async Executor with thread pools
  - Global Configuration system
  - Integration examples (spdlog, NekoThreadPool)
  - Production application patterns

### API Reference

The complete API reference is available directly in the header files:

- **[network.hpp](../include/neko/network/network.hpp)** - Main Network class
  - `execute()` - Synchronous requests
  - `executeAsync()` - Asynchronous requests
  - `executeWithRetry()` - Retry logic
  - `multiThreadedDownload()` - Multi-threaded downloads
  - Utility methods (getContentType, getContentSize, findUrlHeader)

- **[networkTypes.hpp](../include/neko/network/networkTypes.hpp)** - Type definitions
  - `NetworkResult<T>` - Request result structure
  - `RequestConfig` - Request configuration
  - `RetryConfig` - Retry settings
  - `MultiDownloadConfig` - Multi-threaded download settings
  - `RequestType` - Request type enum

- **[networkCommon.hpp](../include/neko/network/networkCommon.hpp)** - Common utilities
  - `header` namespace - HTTP header constants
  - `config::NetConfig` - Global configuration
  - `log::ILogger` - Logger interface
  - `executor::IAsyncExecutor` - Executor interface
  - Helper functions

### Examples

- **[network_test.cpp](../tests/network_test.cpp)** - Comprehensive unit tests that serve as usage examples
  - 27 test cases covering all major features
  - NetworkResult validation
  - Configuration testing
  - Header constants usage

## 🔍 Quick Navigation

### By Topic

#### Basic Usage
- [Installation](../README.md#cmake)
- [Quick Start](../README.md#quick-start)
- [GET Request](../README.md#get-request)
- [POST Request](../README.md#post-request)
- [File Download](../README.md#download-file)

#### Asynchronous Operations
- [Async Requests](../README.md#asynchronous-requests)
- [Custom Executor](ADVANCED_USAGE.md#custom-async-executor)
- [Thread Pool Integration](ADVANCED_USAGE.md#example-1-nekothreadpool-integration)

#### Error Handling
- [Retry Logic](../README.md#retry-logic)
- [Error Handling](../README.md#error-handling)
- [NetworkResult](../README.md#error-handling)

#### Advanced Features
- [Multi-threaded Download](../README.md#multi-threaded-download)
- [Progress Callback](../README.md#progress-callback)
- [Custom Headers](../README.md#custom-headers)
- [Proxy Support](../README.md#proxy-support)

#### Extensibility
- [Custom Logger](ADVANCED_USAGE.md#custom-logger)
- [Custom Executor](ADVANCED_USAGE.md#custom-async-executor)
- [Global Configuration](ADVANCED_USAGE.md#global-configuration)
- [Integration Examples](ADVANCED_USAGE.md#integration-examples)

#### Configuration
- [Global Config](../README.md#global-configuration)
- [Request Config](../README.md#request-configuration)
- [NetConfig API](ADVANCED_USAGE.md#configuration-options)

#### Testing
- [Running Tests](../README.md#testing)
- [Test Examples](../tests/network_test.cpp)
- [CI/CD Setup](../.github/workflows/ci.yml)

## 📖 Reading Path

### For Beginners

1. Start with [README.md](../README.md) for basic concepts
2. Follow the [Quick Start](../README.md#quick-start) guide
3. Try the [Basic Examples](../README.md#usage)
4. Review [Tests](../tests/network_test.cpp) for more examples

### For Intermediate Users

1. Explore [Advanced Features](../README.md#advanced-features)
2. Learn about [Async Operations](../README.md#asynchronous-requests)
3. Understand [Error Handling](../README.md#error-handling)
4. Experiment with [Multi-threaded Downloads](../README.md#multi-threaded-download)

### For Advanced Users

1. Read [ADVANCED_USAGE.md](ADVANCED_USAGE.md) thoroughly
2. Implement [Custom Logger](ADVANCED_USAGE.md#custom-logger)
3. Create [Custom Executor](ADVANCED_USAGE.md#custom-async-executor)
4. Study [Integration Examples](ADVANCED_USAGE.md#integration-examples)
5. Review [Production Patterns](ADVANCED_USAGE.md#complete-example-production-application)

### For Contributors

1. Understand the [Architecture](../include/neko/network/)
2. Review [Test Suite](../tests/network_test.cpp)
3. Check [CI/CD Configuration](../.github/workflows/ci.yml)
4. Read [Troubleshooting Guide](../.github/workflows/Windows_CI_Troubleshooting.md)

## 🎯 Common Tasks

### "I want to make a simple HTTP request"
→ [Basic Example](../README.md#basic-example)

### "I need to download a large file"
→ [Multi-threaded Download](../README.md#multi-threaded-download)

### "I want to use my own logger"
→ [Custom Logger](ADVANCED_USAGE.md#custom-logger)

### "I need to integrate with my thread pool"
→ [Custom Executor](ADVANCED_USAGE.md#custom-async-executor)

### "I want to handle request failures"
→ [Retry Logic](../README.md#retry-logic)

### "I need to track download progress"
→ [Progress Callback](../README.md#progress-callback)

### "I want to configure default settings"
→ [Global Configuration](ADVANCED_USAGE.md#global-configuration)

### "I'm getting build errors on Windows"
→ [Windows CI Troubleshooting](../.github/workflows/Windows_CI_Troubleshooting.md)

## 🔗 External Resources

### Dependencies
- [libcurl Documentation](https://curl.se/libcurl/)
- [OpenSSL Documentation](https://www.openssl.org/docs/)

### Related Projects
- [NekoLog](https://github.com/moehoshio/NekoLog) - Logging library
- [NekoThreadPool](https://github.com/moehoshio/NekoThreadPool) - Thread pool library
- [NekoSchema](https://github.com/moehoshio/NekoSchema) - Type definitions
- [NekoFunction](https://github.com/moehoshio/NekoFunction) - Utility functions
- [NekoSystem](https://github.com/moehoshio/NekoSystem) - System utilities

## 💡 Tips

- **Use Code Examples**: All documentation includes working code examples you can copy-paste
- **Check Tests**: The test suite contains real-world usage patterns
- **Start Simple**: Begin with basic features before moving to advanced topics
- **Use Type Safety**: Leverage C++20 features for compile-time safety
- **Read Headers**: Header files contain detailed API documentation

## 🤝 Contributing

If you find errors in the documentation or want to add examples:

1. Open an issue describing the problem or suggestion
2. Submit a pull request with improvements
3. Ensure code examples are tested and working

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/moehoshio/NekoNet/issues)
- **Discussions**: [GitHub Discussions](https://github.com/moehoshio/NekoNet/discussions)
- **Main README**: [../README.md](../README.md)

---

**Last Updated**: 2025-10-11  
**Version**: 1.0  
**Maintained by**: [moehoshio](https://github.com/moehoshio)
