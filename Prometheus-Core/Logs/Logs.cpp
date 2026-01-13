// Hazno - 2025

#include "Logs.h"

#include <filesystem>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dist_sink.h>

#include "globals.h"

namespace Logs
{
    const std::string logPath       = "Prometheus/Logs/";
    const std::string logExt        = ".txt";

    std::unique_ptr<Detail::Instance> Create_Distributed(const std::string& name, const std::string& fileName)
    {
        auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
        const auto latest = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath + fileName + logExt, true);
        const auto timeStamp = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath + fileName + "_"
            + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + logExt);

        dist_sink->add_sink(latest);
        dist_sink->add_sink(timeStamp);

        return std::make_unique<Detail::Instance>(name, dist_sink);
    }

    void Initialize()
    {
        try {
            spdlog::flush_every(std::chrono::seconds(1));

            Core = Create_Distributed("Prometheus", "Prometheus_Prod");
            spdlog::set_default_logger(Core->Default);
            LOG_CORE(Info, "ACCESSO NO AUTORIZADO: Protocolo Sombra");

            printf("Log Status: sliving\n");
        } catch (const spdlog::spdlog_ex& ex) {
            printf("Log Status: chop + flop. Reason: %s\n", ex.what());
        }
    }

    void Uninitialize()
    {
        if (!Core) {
            return;
        }

        Core->Flush();
        spdlog::shutdown();
    }
}
