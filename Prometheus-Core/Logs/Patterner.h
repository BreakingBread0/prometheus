// Hazno - 2026

#pragma once

#include <spdlog/spdlog.h>

class logger_patterner final : public spdlog::formatter
{
    std::unique_ptr<formatter> default_pattern;
    std::unordered_map<std::string, std::unique_ptr<formatter>> patterns;

    public:
        explicit logger_patterner(std::string default_pattern)
            : default_pattern(std::make_unique<spdlog::pattern_formatter>(std::move(default_pattern)))
        {}

        void pattern_logger(const std::shared_ptr<spdlog::logger>& logger, const std::string& pattern)
        {
            patterns.emplace(std::move(logger->name()),
                std::make_unique<spdlog::pattern_formatter>(std::move(pattern)));
        }

        void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override
        {
            if (const auto it = patterns.find(std::string(msg.logger_name.data(), msg.logger_name.size())); it != patterns.end()) {
                it->second->format(msg, dest);
            } else {
                default_pattern->format(msg, dest);
            }
        }

        std::unique_ptr<formatter> clone() const override
        {
            auto out = std::make_unique<logger_patterner>("%v");
            out->default_pattern = default_pattern->clone();
            out->patterns.reserve(patterns.size());
            for (auto& [k, v] : patterns) {
                out->patterns.emplace(k, v->clone());
            }
            return out;
        }
};