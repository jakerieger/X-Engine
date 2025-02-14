#pragma once
#pragma warning(disable: 4996)

#include <chrono>
#include <cstdlib>

/// @brief Deletes both the move/copy assignment operator and constructor
#define X_CLASS_PREVENT_MOVES_COPIES(CLASS_NAME) \
    CLASS_NAME(const CLASS_NAME&) = delete; \
    CLASS_NAME(CLASS_NAME&&) = delete; \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
    CLASS_NAME& operator=(CLASS_NAME&&) = delete;

/// @brief Deletes the copy constructor and assignment operator
#define X_CLASS_PREVENT_COPIES(CLASS_NAME) \
    CLASS_NAME(const CLASS_NAME&) = delete; \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete;

/// @brief Deletes the move constructor and assignment operator
#define X_CLASS_PREVENT_MOVES(CLASS_NAME) \
    CLASS_NAME(CLASS_NAME&&) = delete; \
    CLASS_NAME& operator=(CLASS_NAME&&) = delete;

#define X_CAST_DYNAMIC_AS \
    template<typename T> \
    T* As() { return dynamic_cast<T*>(this); }

#define X_NODISCARD [[nodiscard]]

#define X_ARRAY_W_SIZE(arr) (arr), sizeof((arr))

#if defined(X_DISTRIBUTION)

#define X_MAIN int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
#define X_MODULE_HANDLE hInstance
#define X_PANIC(fmt, ...)
#define X_PANIC_ASSERT(cond, fmt, ...)

#else

#define X_MAIN int main(int argc, char* argv[])
#define X_MODULE_HANDLE GetModuleHandleA(nullptr)

#include <comdef.h>

[[noreturn]] inline void Panic(const char* file,
                               int line,
                               const char* func,
                               const char* msg) noexcept {
    char msgFormatted[2048];
    snprintf(msgFormatted,
             2048,
             "%s:%d - Panic in `%s()` :\n - %s\n",
             file,
             line,
             func,
             msg);
    fprintf(stderr, static_cast<const char*>(msgFormatted));
    #ifndef NDEBUG
    OutputDebugStringA(msgFormatted);
    #endif
    std::abort();
}

template<typename... Args>
[[noreturn]] void Panic(const char* file,
                        int line,
                        const char* func,
                        const char* fmt,
                        Args... args) noexcept {
    char msg[1024];
    snprintf(msg, sizeof(msg), fmt, args...);
    Panic(file, line, func, msg);
}

#define X_PANIC(fmt, ...) Panic(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);
#define X_PANIC_ASSERT(cond, fmt, ...)                                                                             \
            if (!(cond)) Panic(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);

#endif

#include <fstream>
#include <iostream>

class Logger;
Logger& GetLogger();

#define X_LOG_SEVERITY_INFO 0
#define X_LOG_SEVERITY_WARN 1
#define X_LOG_SEVERITY_ERROR 2
#define X_LOG_SEVERITY_FATAL 3
#define X_LOG_SEVERITY_DEBUG 4

class Logger {
    friend Logger& GetLogger();
    X_CLASS_PREVENT_MOVES_COPIES(Logger)

public:
    ~Logger() {
        if (_logFile.is_open()) {
            _logFile.close();
        }
    }

    void Log(const uint32_t severity, const char* msg) {
        const auto severityStr = GetSeverityString(severity);
        const auto timestamp   = GetTimestamp();
        const auto logEntry    = std::format("{} [{}] - {}\n", timestamp, severityStr, msg);
        _logFile << logEntry;

        #ifndef X_DISTRIBUTION
        std::cout << logEntry;
        #endif

        if (severity == X_LOG_SEVERITY_FATAL) {
            std::abort();
        }
    }

    template<typename... Args>
    void Log(const uint32_t severity, const char* fmt, Args... args) {
        char msg[1024];
        snprintf(msg, sizeof(msg), fmt, args...);
        Log(severity, msg);
    }

private:
    std::ofstream _logFile;
    Logger() : _logFile(GetLogFileName(), std::ios::app) {}

    X_NODISCARD std::string GetTimestamp() const {
        using namespace std::chrono;
        const auto now  = system_clock::now();
        const auto time = system_clock::to_time_t(now);
        char timeBuffer[512];
        ctime_s(timeBuffer, sizeof(timeBuffer), &time);
        std::string timestamp = timeBuffer;
        timestamp.pop_back(); // remove newline
        return timestamp;
    }

    X_NODISCARD std::string GetSeverityString(const uint32_t severity) const {
        if (severity == X_LOG_SEVERITY_INFO)
            return "INFO";
        if (severity == X_LOG_SEVERITY_WARN)
            return "WARNING";
        if (severity == X_LOG_SEVERITY_ERROR)
            return "ERROR";
        if (severity == X_LOG_SEVERITY_FATAL)
            return "FATAL";
        if (severity == X_LOG_SEVERITY_DEBUG)
            return "DEBUG";
        return "";
    }

    X_NODISCARD std::string GetLogFileName() const {
        using namespace std::chrono;
        const auto now        = system_clock::now();
        const auto duration   = now.time_since_epoch();
        const auto unixMillis = duration_cast<milliseconds>(duration).count();
        std::string name      = "Engine_" + std::to_string(unixMillis) + ".log";
        return name;
    }
};

inline Logger& GetLogger() {
    static Logger instance;
    return instance;
}

#define X_LOG(severity, fmt, ...) GetLogger().Log(severity, fmt, ##__VA_ARGS__);
#define X_LOG_INFO(fmt, ...) GetLogger().Log(X_LOG_SEVERITY_INFO, fmt, ##__VA_ARGS__);
#define X_LOG_WARN(fmt, ...) GetLogger().Log(X_LOG_SEVERITY_WARN, fmt, ##__VA_ARGS__);
#define X_LOG_ERROR(fmt, ...) GetLogger().Log(X_LOG_SEVERITY_ERROR, fmt, ##__VA_ARGS__);
#define X_LOG_DEBUG(fmt, ...) GetLogger().Log(X_LOG_SEVERITY_DEBUG, fmt, ##__VA_ARGS__);

/// @brief Fatal log level will also abort program after logging entry!
#define X_LOG_FATAL(fmt, ...) GetLogger().Log(X_LOG_SEVERITY_FATAL, fmt, ##__VA_ARGS__);