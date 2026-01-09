// Hazno - 2025

#include "Logs.h"

#include <filesystem>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Logs
{
    const std::string logPath       = "Prometheus/Logs/";
    const std::string logExt        = ".txt";

    std::shared_ptr<spdlog::logger> CreateDistributedSink(const std::string& name, const std::string& fileName)
    {
        auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
        const auto latest = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath + fileName + logExt, true);
        const auto timeStamp = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath + fileName + "_"
            + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + logExt);

        dist_sink->add_sink(latest);
        dist_sink->add_sink(timeStamp);

        const auto logger = std::make_shared<spdlog::logger>(name, dist_sink);

        logger->set_pattern("%v");
        logger->info("====================================================================");
        logger->info("-   ");
        logger->info("-   {} Log Initialized 💅 - v{}", name, VERSION_STRING);
        logger->info("-   ");
        logger->info("====================================================================");
        logger->info(" ");

        logger->set_pattern("[%T:%e] %-12!s%4# | %-3!l << %v");
        return logger;
    }

    void Initialize()
    {
        try {
            spdlog::flush_every(std::chrono::seconds(1));

            const auto coreLog = CreateDistributedSink("Prometheus", "Prometheus_Prod");
            spdlog::set_default_logger(coreLog);
            SPDLOG_INFO("ACCESSO NO AUTORIZADO: Protocolo Sombra");

            printf("Log Status: sliving\n");
        } catch (const spdlog::spdlog_ex& ex) {
            printf("Log Status: chop + flop. Reason: %s\n", ex.what());
        }
    }

    void Uninitialize()
    {
        spdlog::shutdown();
    }
}
