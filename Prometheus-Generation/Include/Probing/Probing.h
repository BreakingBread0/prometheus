// Hazno - 2026

#pragma once

#include "Probe_STU.h"
#include "AtlasExt/Utility/Modules.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace Generation::Probing
{
    struct ProbeResult
    {
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ProbeResult, STU)

        std::vector<ProbeData_STUProbe> STU;

        ProbeResult() = default;

        explicit ProbeResult(const std::string& path)
        {
            LoadFromFile(path);
        }

        explicit ProbeResult(const Atlas::Utility::Modules::ModuleBounds& module)
        {
            STU = ProbeData_STUProbe::RunProbe(module);
        }

        void SaveToFile(const std::string& path)
        {
            std::filesystem::create_directories(std::filesystem::path(path).parent_path());

            const nlohmann::ordered_json j = *this;
            std::ofstream file(path, std::ios::trunc);
            file << j.dump(4);
        }

        void LoadFromFile(const std::string& path)
        {
            if (!std::filesystem::exists(path)) {
                return;
            }
            std::ifstream file(path, std::ios::in);

            //ignore this error it's a fraud + liar + ugly. clangd was a mistake JUST USE THE NOLINT ???????
            nlohmann::ordered_json jf = nlohmann::ordered_json::parse(file); // NOLINT
            *this = jf.get<ProbeResult>();
        }
    };

    inline void ProbeToFile(const Atlas::Utility::Modules::ModuleBounds& module, const std::string& path, const bool overwrite = false)
    {
        if (!overwrite && std::filesystem::exists(path)) {
            return;
        }

        ProbeResult result(module);
        result.SaveToFile(path);
    }
}
