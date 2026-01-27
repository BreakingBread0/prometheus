// Hazno - 2026

#pragma once
#include <string>

#include "Atlas/Common.h"

// TODO: MIGRATE
// EXTREMELY TEMPORARY HASHING SOLUTION. WILL BE REPLACED LATER W/ EXISTING SOLUTION!
namespace Atlas::Utility::Hash
{
    inline uint32 StringHash(const char* input);

    const char* GetStringForHash(uint32 hash, const char* defaultValue = nullptr);

    void ParseStrings(const std::string& path);
}
