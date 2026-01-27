// Hazno - 2026

#pragma once

#include "Atlas/Common.h"
#include "AtlasExt/Utility/Modules.h"
#include "Atlas/STU/RTTI/STURegistry.h"

#include <vector>
#include <nlohmann/json.hpp>

#include "Probe_Hex.h"
#include "AtlasExt/Utility/Hash.h"

namespace Generation::Probing
{
    struct ProbeData_STUArgument
    {
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ProbeData_STUArgument, Name, Hash, Offset, TypeName, TypeHash, TypeFlag)

        std::string     Name;

        ProbeData_Hex   Hash;
        ProbeData_Hex   Offset;

        std::string     TypeName;
        ProbeData_Hex   TypeHash;
        ProbeData_Hex   TypeFlag;
    };

    struct ProbeData_STUProbe
    {
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ProbeData_STUProbe, Name, Hash, ParentHash, RVA, RegistryRVA, Size, IsArray, ArgumentCount, Arguments)

        std::string     Name;

        ProbeData_Hex   Hash;
        ProbeData_Hex   ParentHash;

        ProbeData_Hex   RVA;
        ProbeData_Hex   RegistryRVA;

        uint64  Size;

        int32   ArgumentCount;
        std::vector<ProbeData_STUArgument> Arguments;

        bool    IsArray;

        static std::vector<ProbeData_STUProbe> RunProbe(Atlas::Utility::Modules::ModuleBounds module)
        {
            std::vector<ProbeData_STUProbe> results{};
            auto registry = STURegistry::Get(module);
            if (!registry) {
                throw std::runtime_error("Atlas RTTI Module Registry is unavailable");
            }

            for (auto [info, reg] : registry)
            {
                ProbeData_STUProbe data{};
                data.Name           = info->Name_str && info->Name_str[0] != '\0' ? info->Name_str : Atlas::Utility::Hash::GetStringForHash(info->Hash, "<unknown>");

                data.Hash           = info->Hash;
                data.ParentHash     = info->Parent ? info->Parent->Hash : 0;

                data.RVA            = module.RVA(info);
                data.RegistryRVA    = module.RVA(reg);

                data.Size           = info->InstanceSize;

                data.IsArray        = info->IsArrayInt == 0;

                data.ArgumentCount  = info->ArgsCount;

                for (const auto arg : reg->Info->RangeArgs(false) | std::views::values)
                {
                    ProbeData_STUArgument argData{};
                    argData.Name        = Atlas::Utility::Hash::GetStringForHash(arg->Hash, "<unknown>");

                    argData.Hash     = arg->Hash;
                    argData.Offset   = arg->Offset;

                    if (arg->Constraint)
                    {
                        argData.TypeName    = Atlas::Utility::Hash::GetStringForHash(arg->Constraint->GetSTUType(), "<unknown>");
                        argData.TypeHash = arg->Constraint->GetSTUType();
                        argData.TypeFlag = arg->Constraint->ToConstraintType();
                    }

                    data.Arguments.push_back(argData);
                }

                results.push_back(data);
            }

            return results;
        }
    };
}
