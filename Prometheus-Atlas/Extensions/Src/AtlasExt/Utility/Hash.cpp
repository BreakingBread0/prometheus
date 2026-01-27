// Hazno - 2026

#include "Hash.h"

#include <fstream>
#include <map>

#include "Modules.h"
#include "Atlas/Common.h"

namespace Atlas::Utility::Hash
{
	std::map<uint64, std::string> hashes{};

    uint32 StringHash(const char* input)
    {
        return Modules::ProgramBounds().VA<uint32(__fastcall*)(const char*)>(0x7f88e0)(input);
    }

    const char* GetStringForHash(uint32 hash, const char* defaultValue)
    {
        if (hashes.find(hash) != hashes.end()) {
            return hashes[hash].c_str();
        }

        return defaultValue;
    }

    void ParseStrings(const std::string& path)
    {
        hashes.clear();

        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            return;
        }

        //parse every string line
        std::string line;
        while (std::getline(file, line))
        {
            //strip newline characters
            std::erase(line, '\n');
            std::erase(line, '\r');
            uint64 hash = StringHash(line.c_str());
            if (hashes[hash] != "") {
                printf("Hash collision detected for hash %llx: '%s' and '%s'\n", hash, hashes[hash].c_str(), line.c_str());
            }

            hashes[hash] = line;
        }
    }
}
