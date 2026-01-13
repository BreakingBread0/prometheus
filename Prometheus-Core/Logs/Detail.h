// Hazno - 2026

#pragma once

#define SPDLOG_SHORT_LEVEL_NAMES \
    { "TRC", "DBG", "INF", "WRN", "ERR", "!!!", "OFF" }

#include <spdlog/spdlog.h>

//
// Avoid including this file directly, include "Logs.h" instead.
// Do not use the Macros here, use the LOG macros defined in Logs.h.
//

#define ILOG(logger, type, lvl, ...)                                                    \
    SPDLOG_LOGGER_CALL(Logs::logger->type, static_cast<spdlog::level::level_enum>(Logs::Detail::LogLevel::lvl), __VA_ARGS__)

#define ILOG_STACK(logger, lvl, ...)                                                    \
    __VA_OPT__(ILOG(logger, Default, lvl, __VA_ARGS__);)                                \
    Logs::logger->LogCallstack(Logs::Detail::LogLevel::lvl)

#define ILOG_EXCEPTION(logger, exception, ...)                                          \
    Logs::logger->Newline();                                                            \
    ILOG(logger, Nested, Critical, "***** Exception Encountered *****");                \
    __VA_OPT__(ILOG(logger, Nested, Critical, __VA_ARGS__);)                            \
    Logs::logger->LogException(exception);                                              \
    ILOG_STACK(logger, Critical);                                                       \
    ILOG(logger, Nested, Critical, "********* Exception End *********");                \
    Logs::logger->Newline();

namespace Logs::Detail
{
    enum LogLevel : int {
        Trace = SPDLOG_LEVEL_TRACE,
        Debug = SPDLOG_LEVEL_DEBUG,
        Info = SPDLOG_LEVEL_INFO,
        Warn = SPDLOG_LEVEL_WARN,
        Error = SPDLOG_LEVEL_ERROR,
        Critical = SPDLOG_LEVEL_CRITICAL,
    };

    struct Instance
    {
        const std::string Name;

        // Logger with default formatting.
        const std::shared_ptr<spdlog::logger> Default;

        // Logger with nested formatting. Useful for array/table-like logs, and for callstacks.
        const std::shared_ptr<spdlog::logger> Nested;

        // Logger without any preformatted information. Use sparingly!
        const std::shared_ptr<spdlog::logger> Clean;

        Instance(const std::string& name, const std::shared_ptr<spdlog::sinks::sink>& sink);

        void Flush() const;

        void LogException(const _EXCEPTION_RECORD* record) const;
        void LogException(const _CONTEXT* ctx) const;
        void LogException(const _EXCEPTION_POINTERS* ex) const;
        void LogCallstack(LogLevel level) const;

        void Newline() const;
    };
}
