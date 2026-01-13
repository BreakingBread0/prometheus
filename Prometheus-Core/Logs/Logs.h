// Hazno - 2025

#pragma once

#include "Detail.h"
#include "Formatting.h"

//  ------------------------------------------------------------------------------
//  LOGGERS
//  ------------------------------------------------------------------------------
//
//  USAGE:
//      LOG_<MODULE>[_<TYPE>](level, ...)
//
//  MODULES:
//   Each module represents a different logger context, printing to a different file.
//   Use the appropriate module for your log messages.
//
//   -  Core - General logs for prometheus-core
//
//
//  TYPES:
//   Each type alters the log format and/or the data included.
//
//   -      Default - Standard log format
//   -  n = Nested (array) - Tweaked formatting for logging arrays/nested data
//   -  s = Stack trace - Captures and logs the current call stack
//   -  e = Exception - Logs a provided exception
//

#define LOG_CORE(lvl, ...)           ILOG            (Core,  Default,    lvl,    __VA_ARGS__)
#define LOG_CORE_n(lvl, ...)         ILOG            (Core,  Nested,     lvl,    __VA_ARGS__)
#define LOG_CORE_s(lvl, ...)         ILOG_STACK      (Core,              lvl,    __VA_ARGS__)
#define LOG_CORE_e(exception, ...)   ILOG_EXCEPTION  (Core,  exception,          __VA_ARGS__)

namespace Logs
{
    inline std::unique_ptr<Detail::Instance> Core{};

    void Initialize();
    void Uninitialize();
}