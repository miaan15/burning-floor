module;

#include <cstdarg>

export module log;

import std;

export enum LogLevel {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERR,
    LOG_CRITICAL
};

inline const char* level_str(LogLevel level) {
    switch (level) {
        case LOG_TRACE:    return "\033[36m"   "TRACE"    "\033[0m";
        case LOG_DEBUG:    return "\033[34m"   "DEBUG"    "\033[0m";
        case LOG_INFO:     return "\033[32m"   "INFO"     "\033[0m";
        case LOG_WARN:     return "\033[33m"   "WARN"     "\033[0m";
        case LOG_ERR:      return "\033[31m"   "ERROR"    "\033[0m";
        case LOG_CRITICAL: return "\033[1;31m" "CRITICAL" "\033[0m";
        default:           return              "UNKNOWN";
    }
}

export void log(LogLevel level, const char* format, va_list args) {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto timer = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::tm bt = *std::localtime(&timer); 

    std::printf("[%02d:%02d:%02d.%03d] [%-8s] ", 
           bt.tm_hour, bt.tm_min, bt.tm_sec, (int)ms.count(), 
           level_str(level));

    std::vprintf(format, args);

    std::printf("\n");
}

export void log_trace(const char* format, ...) {
    va_list args; va_start(args, format);
    log(LOG_TRACE, format, args);
    va_end(args);
}

export void log_debug(const char* format, ...) {
    va_list args; va_start(args, format);
    log(LOG_DEBUG, format, args);
    va_end(args);
}

export void log_info(const char* format, ...) {
    va_list args; va_start(args, format);
    log(LOG_INFO, format, args);
    va_end(args);
}

export void log_warn(const char* format, ...) {
    va_list args; va_start(args, format);
    log(LOG_WARN, format, args);
    va_end(args);
}

export void log_err(const char* format, ...) {
    va_list args; va_start(args, format);
    log(LOG_ERR, format, args);
    va_end(args);
}

export void log_critical(const char* format, ...) {
    va_list args; va_start(args, format);
    log(LOG_CRITICAL, format, args);
    va_end(args);
}
