#pragma once

// Neko Module
#include <neko/schema/types.hpp>

#if __has_include(<neko/log/nlog.hpp>)
#define NEKO_IMPORT_NLOG
#include <neko/log/nlog.hpp>
#endif

// C++ STL
#include <string>
#include <vector>

#include <functional>
#include <memory>
#include <optional>

#include <fstream>
#include <iostream>

#include <future> // For std::async
#include <mutex>
#include <shared_mutex>

namespace neko::network {
    
    namespace header {

        constexpr neko::cstr jsonContentType = "application/json";
        constexpr neko::cstr textContentType = "text/plain";
        constexpr neko::cstr multipartContentType = "multipart/form-data";
        constexpr neko::cstr xmlContentType = "application/xml";
        constexpr neko::cstr htmlContentType = "text/html";
        constexpr neko::cstr pngContentType = "image/png";
        constexpr neko::cstr jpegContentType = "image/jpeg";
        constexpr neko::cstr gifContentType = "image/gif";
        constexpr neko::cstr svgContentType = "image/svg+xml";

        constexpr neko::cstr jsonContentHeader = "Content-Type: application/json";
        constexpr neko::cstr textContentHeader = "Content-Type: text/plain";
        constexpr neko::cstr multipartContentHeader = "Content-Type: multipart/form-data";
        constexpr neko::cstr xmlContentHeader = "Content-Type: application/xml";
        constexpr neko::cstr htmlContentHeader = "Content-Type: text/html";
        constexpr neko::cstr pngContentHeader = "Content-Type: image/png";
        constexpr neko::cstr jpegContentHeader = "Content-Type: image/jpeg";
        constexpr neko::cstr gifContentHeader = "Content-Type: image/gif";
        constexpr neko::cstr svgContentHeader = "Content-Type: image/svg+xml";
    } // namespace header

    namespace config {

        class NetConfig {
            std::string userAgent;
            std::string proxy;
            std::string protocol;
            std::vector<std::string> availableHostList;
            mutable std::shared_mutex mutex;

        public:
            std::string getUserAgent() const {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return userAgent;
            }
            std::string getProxy() const {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return proxy;
            }
            std::string getProtocol() const {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return protocol;
            }
            std::string getAvailableHost() const {
                std::shared_lock<std::shared_mutex> lock(mutex);
                if (!availableHostList.empty()) {
                    return availableHostList.front();
                }
                return std::string();
            }

            NetConfig &setUserAgent(const std::string &ua) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                userAgent = ua;
                return *this;
            }
            NetConfig &setProxy(const std::string &p) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                proxy = p;
                return *this;
            }
            NetConfig &setProtocol(const std::string &p) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                protocol = p;
                return *this;
            }
            NetConfig &setAvailableHostList(const std::vector<std::string> &hosts) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                availableHostList = hosts;
                return *this;
            }
            void pushAvailableHost(const std::string &host) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                availableHostList.push_back(host);
            }
            void clearAvailableHost() {
                std::unique_lock<std::shared_mutex> lock(mutex);
                availableHostList.clear();
            }
            void clear() {
                std::unique_lock<std::shared_mutex> lock(mutex);
                userAgent.clear();
                proxy.clear();
                protocol.clear();
                availableHostList.clear();
            }
        } inline globalConfig;

    } // namespace config

    namespace helper {

        /**
         * @brief Get system proxy settings
         * @return The system proxy address, or std::nullopt if no proxy is set.
         * @example "http://proxy.example.com:8080"
         **/
        std::optional<std::string> getSysProxy();

        template <typename T>
        struct WriteCallbackContext {
            T *buffer;
            std::function<void(neko::uint64)> *progressCallback;
            neko::uint64 totalBytes = 0;
        };

        template <typename T>
        neko::uint64 writeToCallback(char *ptr, neko::uint64 size, neko::uint64 nmemb, void *userdata) {
            auto *ctx = static_cast<WriteCallbackContext<T> *>(userdata);
            neko::uint64 written = size * nmemb;
            ctx->buffer->append(ptr, written);
            ctx->totalBytes += written;
            if (ctx->progressCallback && *ctx->progressCallback) {
                (*ctx->progressCallback)(ctx->totalBytes);
            }
            return written;
        }

        template <>
        inline neko::uint64 writeToCallback<std::string>(char *ptr, neko::uint64 size, neko::uint64 nmemb, void *userdata) {
            auto *ctx = static_cast<WriteCallbackContext<std::string> *>(userdata);
            neko::uint64 written = size * nmemb;
            ctx->buffer->append(ptr, written);
            ctx->totalBytes += written;
            if (ctx->progressCallback && *ctx->progressCallback) {
                (*ctx->progressCallback)(ctx->totalBytes);
            }
            return written;
        }
        template <>
        inline neko::uint64 writeToCallback<std::fstream>(char *ptr, neko::uint64 size, neko::uint64 nmemb, void *userdata) {
            auto *ctx = static_cast<WriteCallbackContext<std::fstream> *>(userdata);
            neko::uint64 written = size * nmemb;
            ctx->buffer->write(ptr, written);
            ctx->totalBytes += written;
            if (ctx->progressCallback && *ctx->progressCallback) {
                (*ctx->progressCallback)(ctx->totalBytes);
            }
            return written;
        }
        template <>
        inline neko::uint64 writeToCallback<std::vector<char>>(char *ptr, neko::uint64 size, neko::uint64 nmemb, void *userdata) {
            auto *ctx = static_cast<WriteCallbackContext<std::vector<char>> *>(userdata);
            neko::uint64 written = size * nmemb;
            ctx->buffer->insert(ctx->buffer->end(), ptr, ptr + written);
            ctx->totalBytes += written;
            if (ctx->progressCallback && *ctx->progressCallback) {
                (*ctx->progressCallback)(ctx->totalBytes);
            }
            return written;
        }

        /**
         * @brief Header callback function for capturing HTTP headers
         * This is used specifically for HEAD requests to capture response headers
         */
        inline neko::uint64 headerCallback(char *ptr, neko::uint64 size, neko::uint64 nmemb, void *userdata) {
            auto *headers = static_cast<std::string *>(userdata);
            neko::uint64 dataSize = size * nmemb;
            headers->append(ptr, dataSize);
            return dataSize;
        }

    } // namespace helper

    namespace log {
        // Logging functions

        class ILogger {
        public:
            virtual ~ILogger() = default;
            virtual void error(const std::string &msg) = 0;
            virtual void info(const std::string &msg) = 0;
            virtual void warn(const std::string &msg) = 0;
            virtual void debug(const std::string &msg) = 0;
        };

        class DefaultLogger : public ILogger {
        public:
            void error(const std::string &msg) override {
                std::cerr << "Network Error: " << msg << std::endl;
            }
            void info(const std::string &msg) override {
                std::cout << "Network Info: " << msg << std::endl;
            }
            void warn(const std::string &msg) override {
                std::cerr << "Network Warning: " << msg << std::endl;
            }
            void debug(const std::string &msg) override {
                std::cout << "Network Debug: " << msg << std::endl;
            }
        };

#ifdef NEKO_IMPORT_NLOG
        class NLogLogger : public ILogger {
        public:
            void error(const std::string &msg) override { neko::log::error(msg); }
            void info(const std::string &msg) override { neko::log::info(msg); }
            void warn(const std::string &msg) override { neko::log::warn(msg); }
            void debug(const std::string &msg) override { neko::log::debug(msg); }
        };
#endif // NEKO_IMPORT_NLOG

        // Logger factory type
        using LoggerFactory = std::function<std::shared_ptr<ILogger>()>;

        /**
         * @brief Get the global logger factory
         * 
         * If NekoLog is available, it uses NLogLogger by default; otherwise, it falls back to DefaultLogger.
         * 
         * @note This function ensures that the factory is initialized only once in a thread-safe manner.
         */
        inline LoggerFactory &getLoggerFactory() {
            static LoggerFactory factory = []() -> std::shared_ptr<ILogger> {
#ifdef NEKO_IMPORT_NLOG
                return std::make_shared<NLogLogger>();
#else
                return std::make_shared<DefaultLogger>();
#endif
            };
            return factory;
        }

        /**
         * 
         * @brief Set a custom logger factory
         * @param factory The custom logger factory function
         * @example
         *    neko::network::log::setLoggerFactory([]() {
         *        return std::make_shared<MyCustomLogger>();
         *    });
         */
        inline void setLoggerFactory(LoggerFactory factory) {
            getLoggerFactory() = std::move(factory);
        }

        /// @brief Create a logger instance using the registered factory
        /// @return A shared pointer to the logger instance
        inline std::shared_ptr<ILogger> createLogger() {
            return getLoggerFactory()();
        }

    } // namespace log

    namespace executor {

        // Asynchronous executor interface
        class IAsyncExecutor {
        public:
            virtual ~IAsyncExecutor() = default;

            template <typename F>
            auto submit(F &&f) -> std::future<decltype(f())> {
                return submitImpl(std::function<decltype(f())()>(std::forward<F>(f)));
            }

        protected:
            template <typename R>
            std::future<R> submitImpl(std::function<R()> f) {
                // Default implementation using std::async
                return std::async(std::launch::async, std::move(f));
            }
        };

        class StdAsyncExecutor : public IAsyncExecutor {
        protected:
            template <typename R>
            std::future<R> submitImpl(std::function<R()> f) {
                return std::async(std::launch::async, std::move(f));
            }
        };

        // Executor factory type
        using ExecutorFactory = std::function<std::shared_ptr<IAsyncExecutor>()>;

        /**
         * @brief Get the global executor factory
         * @return The global executor factory
         */
        inline ExecutorFactory &getExecutorFactory() {
            static ExecutorFactory factory = []() -> std::shared_ptr<IAsyncExecutor> {
                return std::make_shared<StdAsyncExecutor>();
            };
            return factory;
        }

        /**
         * @brief Set a custom executor factory
         * @param factory The custom executor factory function
         * @example
         *    neko::network::executor::setExecutorFactory([]() {
         *        return std::make_shared<MyCustomExecutor>();
         *    });
         */
        inline void setExecutorFactory(ExecutorFactory factory) {
            getExecutorFactory() = std::move(factory);
        }

        // Factory function to create an executor
        inline std::shared_ptr<IAsyncExecutor> createExecutor() {
            return getExecutorFactory()();
        }

    } // namespace executor

    /**
     * @brief Build a complete URL from the given path, host, and protocol.
     * @param path The path to append to the host.
     * @param host The host to use (default is globalConfig.host).
     * @param protocol The protocol to use (default is globalConfig.protocol).
     * @return std::string - The complete URL.
     */
    inline std::string buildUrl(const std::string &path, const std::string &host = config::globalConfig.getAvailableHost(), const std::string &protocol = config::globalConfig.getProtocol()) {
        return (protocol + host + path);
    }

    /**
     * @brief Initialize the network module
     **/
    inline void initialize(std::function<void(config::NetConfig &)> updateNetCfg) {
        if (updateNetCfg) {
            updateNetCfg(config::globalConfig);
        } else {
            config::globalConfig
                .setProtocol("https://")
                .setUserAgent("NekoNetwork/v1.0.2 +https://github.com/moehoshio/NekoNetwork")
                .setProxy("true");
        }
    }

} // namespace neko::network
