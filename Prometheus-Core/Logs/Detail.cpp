// Hazno - 2026

#include "Detail.h"

#include "Common.h"
#include "Patterner.h"
#include "globals.h"
#include "Utility/Exception.h"
#include "Utility/Modules.h"

namespace Logs::Detail
{
    const std::string logPath       = "Prometheus/Logs/";
    const std::string logExt        = ".txt";

    Instance::Instance(const std::string& name, const std::shared_ptr<spdlog::sinks::sink>& sink)
        :   Name(name),
            Default(std::make_shared<spdlog::logger>(name, sink)),
            Nested(std::make_unique<spdlog::logger>(name + "_NESTED", sink)),
            Clean(std::make_unique<spdlog::logger>(name + "_CLEAN", sink))
    {

#if _DEBUG
        auto patterner = std::make_unique<logger_patterner>("[%T:%e] [%L] %-16!s%4# << %v");
        patterner->pattern_logger(Nested,"[%T:%e] [%L] %-14!s%4# << -  %v");
        patterner->pattern_logger(Clean,"%v");
#else
        auto patterner = std::make_unique<logger_patterner>("[%T:%e] [%L] %-10!s << %v");
        patterner->pattern_logger(Nested, "[%T:%e] [%L]         << -  %v");
        patterner->pattern_logger(Clean, "%v");
#endif

        sink->set_formatter(std::move(patterner));

        Clean->info("====================================================================");
        Clean->info("-   ");
        Clean->info("-   {} Log Initialized 💅 - v{} ({} Build)", name, VERSION_STRING, BUILD_TYPE);
        Clean->info("-   ");
        Clean->info("====================================================================");
        Clean->info("-   ");
        Clean->info("-   Game Base:   {:n} {:n}", FmtAddr(GameBounds().Base()), FmtAddr(GameBounds().Base()));
        Clean->info("-   Module Base: {:n} {:n}", FmtAddr(CoreBounds().Base()), FmtAddr(CoreBounds().Size()));
        Clean->info("-   Main Thread: {}", GetCurrentThreadId());
        Clean->info("-   ");
        Clean->info("====================================================================");
        Clean->info(" ");
    }

    void Instance::Flush() const
    {
        if (!Default) {
            return;
        }

        Default->flush();
        Nested->flush();
        Clean->flush();
    }

    void Instance::LogException(const _EXCEPTION_RECORD* record) const
    {
        Nested->critical("Win32 Exception raised at {}", FmtAddr(record->ExceptionAddress));

        Newline();
        Nested->critical("***  General Information");
        Nested->critical("Desc:   \"{}\"", Utility::Exception::W32ErrorToString(record->ExceptionCode));
        Nested->critical("Thread: {0:n}     Flags:  {1:08b} ({1:0>4})", FmtAddr(GetCurrentThreadId()), record->ExceptionFlags);
        Nested->critical("Code:   {:n}     Params: {}", FmtAddr(record->ExceptionCode), record->NumberParameters);
        for (int i = 0; i < record->NumberParameters; i++) {
            Nested->critical("Param   {0:0>2}: {1:nv}", i, FmtAddr(record->ExceptionInformation[i]));
        }

        if (record->ExceptionRecord) {
            Newline();
            Nested->critical("***  Chained Exception");
            LogException(record->ExceptionRecord);
        }
    }

    void Instance::LogException(const _CONTEXT* ctx) const
    {
        Newline();
        Nested->critical("***  Registers");
        Nested->critical("DR0: {:n}     DR3: {:n}",                 FmtAddr(ctx->Dr0), FmtAddr(ctx->Dr3));
        Nested->critical("DR1: {:n}     DR6: {:n}",                 FmtAddr(ctx->Dr1), FmtAddr(ctx->Dr6));
        Nested->critical("DR2: {:n}     DR7: {:n}",                 FmtAddr(ctx->Dr2), FmtAddr(ctx->Dr7));
        Nested->critical("RAX: {:n}     RBP: {:n}     R10: {:n}",   FmtAddr(ctx->Rax), FmtAddr(ctx->Rbp), FmtAddr(ctx->R10));
        Nested->critical("RCX: {:n}     RSI: {:n}     R11: {:n}",   FmtAddr(ctx->Rcx), FmtAddr(ctx->Rsi), FmtAddr(ctx->R11));
        Nested->critical("RDX: {:n}     RDI: {:n}     R12: {:n}",   FmtAddr(ctx->Rdx), FmtAddr(ctx->Rdi), FmtAddr(ctx->R12));
        Nested->critical("RBX: {:n}      R8: {:n}     R13: {:n}",   FmtAddr(ctx->Rbx), FmtAddr(ctx->R8 ), FmtAddr(ctx->R13));
        Nested->critical("RSP: {:n}      R9: {:n}     R14: {:n}",   FmtAddr(ctx->Rsp), FmtAddr(ctx->R9 ), FmtAddr(ctx->R14));
        Nested->critical("R15: {:n}     RIP: {:n}",                 FmtAddr(ctx->R15), FmtAddr(ctx->Rip));
    }

    void Instance::LogException(const _EXCEPTION_POINTERS* ex) const
    {
        LogException(ex->ExceptionRecord);
        LogException(ex->ContextRecord);
    }

    void Instance::LogCallstack(const LogLevel level) const
    {
        thread_local void* stack[48];

        Newline();
        Nested->critical("***  Callstack");
        const auto lvl = static_cast<spdlog::level::level_enum>(level);
        const auto count = CaptureStackBackTrace(1, 48, stack, nullptr);

        for (USHORT i = 0; i < count; i++) {
            Nested->log(lvl, "Address {:0>2}: {}", i, FmtAddr(stack[i]));
        }

        Flush();
    }

    void Instance::Newline() const
    {
        Clean->info(" ");
    }
}
