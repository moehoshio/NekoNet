# NekoNet Advanced Usage Guide

This guide covers advanced usage patterns for NekoNet, including custom loggers, custom executors, and global configuration.

## Table of Contents

- [Custom Logger](#custom-logger)
- [Custom Async Executor](#custom-async-executor)
- [Global Configuration](#global-configuration)
- [Integration Examples](#integration-examples)

## Custom Logger

### Overview

NekoNet's logging system is designed to be flexible and extensible. By default:
- **NLogLogger** is used if NekoLog is available (automatically detected)
- **DefaultLogger** is used as fallback (stdout/stderr output)

### Implementing a Custom Logger

To create a custom logger, implement the `neko::network::log::ILogger` interface:

```cpp
#include <neko/network/networkCommon.hpp>

class MyCustomLogger : public neko::network::log::ILogger {
public:
    void error(const std::string &msg) override {
        // Your custom error logging implementation
        myLogSystem->log(LogLevel::ERROR, "NekoNet", msg);
    }
    
    void info(const std::string &msg) override {
        // Your custom info logging implementation
        myLogSystem->log(LogLevel::INFO, "NekoNet", msg);
    }
    
    void warn(const std::string &msg) override {
        // Your custom warning logging implementation
        myLogSystem->log(LogLevel::WARNING, "NekoNet", msg);
    }
    
    void debug(const std::string &msg) override {
        // Your custom debug logging implementation
        myLogSystem->log(LogLevel::DEBUG, "NekoNet", msg);
    }

private:
    MyLogSystem* myLogSystem = MyLogSystem::getInstance();
};
```

### Global Logger Factory

Set a global logger factory that will be used by all Network instances:

```cpp
#include <neko/network/network.hpp>

int main() {
    // Set global logger factory
    neko::network::log::setLoggerFactory([]() -> std::shared_ptr<neko::network::log::ILogger> {
        return std::make_shared<MyCustomLogger>();
    });
    
    // All Network instances will now use MyCustomLogger
    neko::network::Network network1;
    neko::network::Network network2;
    
    // Both will log using MyCustomLogger
    network1.execute(config);
    network2.execute(config);
    
    return 0;
}
```

### Per-Instance Logger

Provide a custom logger to a specific Network instance:

```cpp
#include <neko/network/network.hpp>

int main() {
    // Create a custom logger instance
    auto customLogger = std::make_shared<MyCustomLogger>();
    
    // Pass it to Network constructor
    neko::network::Network network(
        neko::network::executor::createExecutor(),  // Default executor
        customLogger                                 // Custom logger
    );
    
    // This instance will use MyCustomLogger
    network.execute(config);
    
    return 0;
}
```

### Logger Integration Examples

#### Example 1: spdlog Integration

```cpp
#include <spdlog/spdlog.h>
#include <neko/network/networkCommon.hpp>

class SpdlogLogger : public neko::network::log::ILogger {
public:
    SpdlogLogger() {
        logger = spdlog::get("nekonet");
        if (!logger) {
            logger = spdlog::stdout_color_mt("nekonet");
        }
    }
    
    void error(const std::string &msg) override {
        logger->error(msg);
    }
    
    void info(const std::string &msg) override {
        logger->info(msg);
    }
    
    void warn(const std::string &msg) override {
        logger->warn(msg);
    }
    
    void debug(const std::string &msg) override {
        logger->debug(msg);
    }

private:
    std::shared_ptr<spdlog::logger> logger;
};

// Usage
int main() {
    neko::network::log::setLoggerFactory([]() {
        return std::make_shared<SpdlogLogger>();
    });
    
    neko::network::Network network;
    // Will log via spdlog
}
```

#### Example 2: File-based Logger

```cpp
#include <fstream>
#include <mutex>
#include <neko/network/networkCommon.hpp>

class FileLogger : public neko::network::log::ILogger {
public:
    FileLogger(const std::string& filename) 
        : logFile(filename, std::ios::app) {}
    
    void error(const std::string &msg) override {
        log("ERROR", msg);
    }
    
    void info(const std::string &msg) override {
        log("INFO", msg);
    }
    
    void warn(const std::string &msg) override {
        log("WARN", msg);
    }
    
    void debug(const std::string &msg) override {
        log("DEBUG", msg);
    }

private:
    void log(const std::string& level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mutex);
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        logFile << "[" << std::ctime(&time) << "] [" << level << "] " << msg << std::endl;
    }
    
    std::ofstream logFile;
    std::mutex mutex;
};

// Usage
int main() {
    neko::network::log::setLoggerFactory([]() {
        return std::make_shared<FileLogger>("nekonet.log");
    });
}
```

## Custom Async Executor

### Overview

NekoNet uses an executor pattern for asynchronous operations. By default, it uses `StdAsyncExecutor` which wraps `std::async`. You can replace this with your own implementation, such as a thread pool.

### Implementing a Custom Executor

To create a custom executor, implement the `neko::network::executor::IAsyncExecutor` interface:

```cpp
#include <neko/network/networkCommon.hpp>

class MyThreadPoolExecutor : public neko::network::executor::IAsyncExecutor {
public:
    MyThreadPoolExecutor(size_t numThreads) 
        : threadPool(numThreads) {}
    
protected:
    template <typename R>
    std::future<R> submitImpl(std::function<R()> f) override {
        // Submit task to your thread pool
        return threadPool.enqueue(std::move(f));
    }

private:
    MyThreadPool threadPool;
};
```

### Global Executor Factory

Set a global executor factory that will be used by all Network instances:

```cpp
#include <neko/network/network.hpp>

int main() {
    // Set global executor factory
    neko::network::executor::setExecutorFactory([]() -> std::shared_ptr<neko::network::executor::IAsyncExecutor> {
        return std::make_shared<MyThreadPoolExecutor>(8);  // 8 worker threads
    });
    
    // All Network instances will now use MyThreadPoolExecutor
    neko::network::Network network1;
    neko::network::Network network2;
    
    // Both will use the same thread pool type
    auto future1 = network1.executeAsync(config);
    auto future2 = network2.executeAsync(config);
    
    return 0;
}
```

### Per-Instance Executor

Provide a custom executor to a specific Network instance:

```cpp
#include <neko/network/network.hpp>

int main() {
    // Create a custom executor instance
    auto customExecutor = std::make_shared<MyThreadPoolExecutor>(16);
    
    // Pass it to Network constructor
    neko::network::Network network(
        customExecutor,                              // Custom executor
        neko::network::log::createLogger()          // Default logger
    );
    
    // Async operations will use your thread pool
    auto future = network.executeAsync(config);
    
    return 0;
}
```

### Executor Integration Examples

#### Example 1: NekoThreadPool Integration

```cpp
#include <neko/threadpool/thread_pool.hpp>
#include <neko/network/network.hpp>

class NekoThreadPoolExecutor : public neko::network::executor::IAsyncExecutor {
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

// Usage
int main() {
    // Create shared thread pool
    auto pool = std::make_shared<neko::ThreadPool>(
        8,                              // 8 worker threads
        neko::ThreadPoolConfig()
            .setPriority(true)         // Enable priority support
            .setQueueSize(1000)        // Max queue size
    );
    
    // Set global executor
    neko::network::executor::setExecutorFactory([pool]() {
        return std::make_shared<NekoThreadPoolExecutor>(pool);
    });
    
    // Create multiple Network instances
    neko::network::Network network1;
    neko::network::Network network2;
    
    // Execute multiple async requests
    std::vector<std::future<neko::network::NetworkResult<>>> futures;
    
    for (const auto& url : urls) {
        neko::network::RequestConfig config;
        config.url = url;
        
        // Randomly choose which network instance to use
        auto& network = (futures.size() % 2 == 0) ? network1 : network2;
        futures.push_back(network.executeAsync(config));
    }
    
    // Wait for all results
    for (auto& future : futures) {
        auto result = future.get();
        if (result.isSuccess()) {
            std::cout << "Success: " << result.content.substr(0, 100) << "..." << std::endl;
        }
    }
    
    return 0;
}
```

#### Example 2: Custom Priority Thread Pool

```cpp
#include <neko/network/networkCommon.hpp>
#include <queue>
#include <thread>
#include <condition_variable>

class PriorityThreadPool {
public:
    struct Task {
        std::function<void()> func;
        int priority;
        
        bool operator<(const Task& other) const {
            return priority < other.priority;  // Higher priority first
        }
    };
    
    PriorityThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] { workerThread(); });
        }
    }
    
    ~PriorityThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }
    
    template <typename F>
    auto submit(F&& f, int priority = 0) -> std::future<decltype(f())> {
        using return_type = decltype(f());
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
        auto future = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(mutex);
            tasks.push({[task]() { (*task)(); }, priority});
        }
        condition.notify_one();
        
        return future;
    }

private:
    void workerThread() {
        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mutex);
                condition.wait(lock, [this] { return stop || !tasks.empty(); });
                
                if (stop && tasks.empty()) return;
                
                task = tasks.top();
                tasks.pop();
            }
            task.func();
        }
    }
    
    std::vector<std::thread> workers;
    std::priority_queue<Task> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    bool stop;
};

class PriorityExecutor : public neko::network::executor::IAsyncExecutor {
public:
    PriorityExecutor(size_t numThreads, int defaultPriority = 0) 
        : pool(numThreads), priority(defaultPriority) {}
    
    void setPriority(int p) { priority = p; }
    
protected:
    template <typename R>
    std::future<R> submitImpl(std::function<R()> f) override {
        return pool.submit(std::move(f), priority);
    }

private:
    PriorityThreadPool pool;
    int priority;
};

// Usage
int main() {
    auto executor = std::make_shared<PriorityExecutor>(8);
    neko::network::Network network(executor, neko::network::log::createLogger());
    
    // High priority requests
    executor->setPriority(10);
    auto criticalFuture = network.executeAsync(criticalConfig);
    
    // Normal priority requests
    executor->setPriority(5);
    auto normalFuture = network.executeAsync(normalConfig);
    
    // Low priority requests
    executor->setPriority(1);
    auto lowFuture = network.executeAsync(lowConfig);
    
    return 0;
}
```

## Global Configuration

### Overview

NekoNet provides a global configuration system for setting default values that will be used by all `RequestConfig` instances unless explicitly overridden.

### Initialization

Initialize the global configuration at application startup:

```cpp
#include <neko/network/network.hpp>

int main() {
    // Initialize with custom configuration
    neko::network::initialize([](neko::network::config::NetConfig& cfg) {
        cfg.setUserAgent("MyApp/2.0 (+https://myapp.com)")
           .setProtocol("https://")
           .setProxy("http://proxy.example.com:8080")
           .setAvailableHostList({
               "api.example.com",
               "api-backup.example.com",
               "api-failover.example.com"
           });
    });
    
    // Now all requests will use these defaults
    neko::network::Network network;
    
    neko::network::RequestConfig config;
    config.url = "https://api.example.com/data";
    // config.userAgent is automatically "MyApp/2.0 (+https://myapp.com)"
    // config.proxy is automatically "http://proxy.example.com:8080"
    
    network.execute(config);
    
    return 0;
}
```

### Using Default Configuration

If you don't need custom settings, use the default configuration:

```cpp
// Use default configuration
neko::network::initialize(nullptr);

// Defaults:
// - User-Agent: "NekoNet/v1.0 +https://github.com/moehoshio/NekoNet"
// - Protocol: "https://"
// - Proxy: "true" (use system proxy)
```

### Configuration Options

#### User-Agent

Set the default User-Agent for all requests:

```cpp
neko::network::config::globalConfig.setUserAgent("MyApp/1.0");

// All new RequestConfig instances will use this User-Agent
neko::network::RequestConfig config;
config.url = "https://api.example.com/data";
// config.userAgent == "MyApp/1.0"
```

#### Proxy

Configure proxy settings:

```cpp
// Use specific proxy
neko::network::config::globalConfig.setProxy("http://proxy.example.com:8080");

// Use system proxy
neko::network::config::globalConfig.setProxy("true");

// No proxy
neko::network::config::globalConfig.setProxy("");
```

#### Protocol

Set the default protocol:

```cpp
// Use HTTPS by default
neko::network::config::globalConfig.setProtocol("https://");

// Use HTTP by default
neko::network::config::globalConfig.setProtocol("http://");
```

#### Available Host List

Configure multiple hosts for failover:

```cpp
// Set host list
neko::network::config::globalConfig.setAvailableHostList({
    "api-primary.example.com",
    "api-secondary.example.com",
    "api-tertiary.example.com"
});

// Add a host
neko::network::config::globalConfig.pushAvailableHost("api-backup.example.com");

// Get current host
std::string host = neko::network::config::globalConfig.getAvailableHost();
// Returns: "api-primary.example.com"

// Clear host list
neko::network::config::globalConfig.clearAvailableHost();
```

### Building URLs

Use the `buildUrl` helper to construct URLs with global configuration:

```cpp
// Configure global settings
neko::network::initialize([](neko::network::config::NetConfig& cfg) {
    cfg.setProtocol("https://")
       .setAvailableHostList({"api.example.com"});
});

// Build URL using global config
std::string url = neko::network::buildUrl("/users/123");
// Result: "https://api.example.com/users/123"

// Build URL with custom parameters
std::string customUrl = neko::network::buildUrl(
    "/data",                    // path
    "custom.example.com",      // custom host
    "http://"                  // custom protocol
);
// Result: "http://custom.example.com/data"
```

### Thread Safety

All global configuration methods are thread-safe and use `std::shared_mutex` for concurrent access:

```cpp
// Thread 1: Reading
std::string userAgent = neko::network::config::globalConfig.getUserAgent();

// Thread 2: Writing (blocks readers temporarily)
neko::network::config::globalConfig.setUserAgent("NewAgent/1.0");

// Thread 3: Reading (waits for writer to complete)
std::string proxy = neko::network::config::globalConfig.getProxy();
```

### Clearing Configuration

Reset all configuration to defaults:

```cpp
// Clear all settings
neko::network::config::globalConfig.clear();

// Or clear specific settings
neko::network::config::globalConfig.clearAvailableHost();
```

## Integration Examples

### Complete Example: Production Application

```cpp
#include <neko/network/network.hpp>
#include <neko/threadpool/thread_pool.hpp>
#include <spdlog/spdlog.h>

// Custom logger using spdlog
class ProductionLogger : public neko::network::log::ILogger {
public:
    ProductionLogger() {
        logger = spdlog::stdout_color_mt("nekonet");
        logger->set_level(spdlog::level::info);
    }
    
    void error(const std::string &msg) override { logger->error(msg); }
    void info(const std::string &msg) override { logger->info(msg); }
    void warn(const std::string &msg) override { logger->warn(msg); }
    void debug(const std::string &msg) override { logger->debug(msg); }

private:
    std::shared_ptr<spdlog::logger> logger;
};

// Custom executor using NekoThreadPool
class ProductionExecutor : public neko::network::executor::IAsyncExecutor {
public:
    ProductionExecutor(std::shared_ptr<neko::ThreadPool> pool) 
        : threadPool(pool) {}
    
protected:
    template <typename R>
    std::future<R> submitImpl(std::function<R()> f) override {
        return threadPool->submit(std::move(f));
    }

private:
    std::shared_ptr<neko::ThreadPool> threadPool;
};

class Application {
public:
    void initialize() {
        // Setup logging
        neko::network::log::setLoggerFactory([]() {
            return std::make_shared<ProductionLogger>();
        });
        
        // Setup thread pool
        auto pool = std::make_shared<neko::ThreadPool>(16);
        neko::network::executor::setExecutorFactory([pool]() {
            return std::make_shared<ProductionExecutor>(pool);
        });
        
        // Setup global network configuration
        neko::network::initialize([](neko::network::config::NetConfig& cfg) {
            cfg.setUserAgent("MyApp/1.0 (+https://myapp.com)")
               .setProtocol("https://")
               .setAvailableHostList({
                   "api.myapp.com",
                   "api-backup.myapp.com"
               });
        });
    }
    
    void run() {
        neko::network::Network network;
        
        // Make multiple async requests
        std::vector<std::future<neko::network::NetworkResult<>>> futures;
        
        for (const auto& endpoint : endpoints) {
            neko::network::RequestConfig config;
            config.url = neko::network::buildUrl(endpoint);
            futures.push_back(network.executeAsync(config));
        }
        
        // Process results
        for (auto& future : futures) {
            auto result = future.get();
            if (result.isSuccess()) {
                processData(result.content);
            } else {
                handleError(result.errorMessage);
            }
        }
    }

private:
    std::vector<std::string> endpoints = {"/users", "/posts", "/comments"};
    
    void processData(const std::string& data) {
        // Process the data
    }
    
    void handleError(const std::string& error) {
        // Handle the error
    }
};

int main() {
    Application app;
    app.initialize();
    app.run();
    return 0;
}
```

### Example: Microservice with Custom Configuration

```cpp
#include <neko/network/network.hpp>

class MicroserviceClient {
public:
    MicroserviceClient(const std::string& serviceName, const std::string& baseUrl) {
        // Create service-specific logger
        auto logger = std::make_shared<ServiceLogger>(serviceName);
        
        // Create service-specific executor
        auto executor = std::make_shared<ServiceExecutor>(serviceName);
        
        // Create network with custom components
        network = std::make_unique<neko::network::Network>(executor, logger);
        
        this->baseUrl = baseUrl;
    }
    
    neko::network::NetworkResult<> get(const std::string& path) {
        neko::network::RequestConfig config;
        config.url = baseUrl + path;
        config.method = neko::network::RequestType::Get;
        return network->execute(config);
    }
    
    neko::network::NetworkResult<> post(const std::string& path, const std::string& data) {
        neko::network::RequestConfig config;
        config.url = baseUrl + path;
        config.method = neko::network::RequestType::Post;
        config.postData = data;
        config.header = "Content-Type: application/json";
        return network->execute(config);
    }

private:
    std::unique_ptr<neko::network::Network> network;
    std::string baseUrl;
};

int main() {
    // Setup global configuration
    neko::network::initialize([](neko::network::config::NetConfig& cfg) {
        cfg.setUserAgent("Microservice/1.0")
           .setProxy("");  // No proxy for internal services
    });
    
    // Create clients for different services
    MicroserviceClient userService("UserService", "http://users.internal");
    MicroserviceClient orderService("OrderService", "http://orders.internal");
    
    // Use the clients
    auto userResult = userService.get("/users/123");
    auto orderResult = orderService.post("/orders", R"({"item": "widget", "qty": 5})");
    
    return 0;
}
```

## Best Practices

1. **Initialize Once**: Set global logger and executor factories at application startup before creating any Network instances.

2. **Shared Resources**: When using custom executors with thread pools, consider sharing the thread pool across multiple Network instances.

3. **Logger Lifetime**: Ensure custom loggers remain valid for the lifetime of Network instances that use them.

4. **Thread Safety**: Both logger and executor implementations must be thread-safe.

5. **Error Handling**: Custom loggers and executors should handle errors gracefully without throwing exceptions.

6. **Resource Cleanup**: Properly clean up resources (thread pools, log files, etc.) when the application terminates.

7. **Configuration Consistency**: Set global configuration early in the application lifecycle before creating Network instances.

## Troubleshooting

### Problem: Custom logger not being used

**Solution**: Ensure `setLoggerFactory` is called before creating Network instances:

```cpp
// Correct order
neko::network::log::setLoggerFactory([]() { return std::make_shared<MyLogger>(); });
neko::network::Network network;  // Will use MyLogger

// Wrong order
neko::network::Network network;  // Will use default logger
neko::network::log::setLoggerFactory([]() { return std::make_shared<MyLogger>(); });
```

### Problem: Thread pool not being utilized

**Solution**: Verify that `submitImpl` is correctly overridden:

```cpp
protected:
    template <typename R>
    std::future<R> submitImpl(std::function<R()> f) override {  // Note: override keyword
        return threadPool.submit(std::move(f));
    }
```

### Problem: Global configuration not applied

**Solution**: Call `initialize()` before creating RequestConfig instances:

```cpp
neko::network::initialize([](auto& cfg) { cfg.setUserAgent("MyApp"); });
neko::network::RequestConfig config;  // Will have userAgent = "MyApp"
```

## See Also

- [Main README](../README.md)
- [NekoLog Documentation](https://github.com/moehoshio/NekoLog)
- [NekoThreadPool Documentation](https://github.com/moehoshio/NekoThreadPool)
