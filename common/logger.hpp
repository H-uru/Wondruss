#ifndef WONDRUSS_COMMON_LOGGER
#define WONDRUSS_COMMON_LOGGER

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include <sstream>

namespace Wondruss {
  namespace Logger {
    void init(const char*);
    extern log4cplus::Logger logger;
    
    template<typename T>
    void sprintf(std::stringstream& stream, T val) {
      stream << val;
    }

    template<typename T, typename... params>
    void sprintf(std::stringstream& stream, T val, params... args) {
      stream << val;
      sprintf(stream, args...);
    }
    
    template <typename ... params>
    void log(const char* file, int line, log4cplus::LogLevel level, params... args) {
      if (logger.isEnabledFor(level)) {
        std::stringstream stream;
        sprintf(stream, args...);
        logger.forcedLog(level, stream.str().c_str(), file, line);
      }
    }
  }
}

#define LOG_TRACE(...) Wondruss::Logger::log(__FILE__, __LINE__, log4cplus::TRACE_LOG_LEVEL, __VA_ARGS__)
#define LOG_DEBUG(...) Wondruss::Logger::log(__FILE__, __LINE__, log4cplus::DEBUG_LOG_LEVEL, __VA_ARGS__)
#define LOG_INFO(...)  Wondruss::Logger::log(__FILE__, __LINE__, log4cplus::INFO_LOG_LEVEL, __VA_ARGS__)
#define LOG_WARN(...)  Wondruss::Logger::log(__FILE__, __LINE__, log4cplus::WARN_LOG_LEVEL, __VA_ARGS__)
#define LOG_ERROR(...) Wondruss::Logger::log(__FILE__, __LINE__, log4cplus::ERROR_LOG_LEVEL, __VA_ARGS__)
#define LOG_FATAL(...) Wondruss::Logger::log(__FILE__, __LINE__, log4cplus::FATAL_LOG_LEVEL, __VA_ARGS__)

#endif // WONDRUSS_COMMON_LOGGER