#pragma once
#include "STU.h"
#include "Statescript.h"
#include <fstream>
#include <iostream>
#include "stringhash_library.h"
#include "Atlas/STU/RTTI/STURegistry.h"

class hash_exporter {
public:
	static inline void export_hashes() {
		std::ofstream out_file("hashcat.txt", std::ios::trunc);
		
		if (!out_file.is_open()) {
			printf("Failed to open export file!\n");
			return;
		}
		out_file << std::hex;

		STURegistry* reg = STURegistry::Get();
		while (reg) {
			out_file << std::format("{:08x}", reg->Info->Hash) << ":00000000" << std::endl;
			for (int i = 0; i < reg->Info->ArgsCount; i++) {
				out_file << std::format("{:08x}", reg->Info->Args[i].Hash) << ":00000000" << std::endl;
			}
			reg = reg->Next;
		}

		STU_EnumRegistry* ereg = STUEnumRegistry();
		while (ereg) {
			out_file << std::format("{:08x}", ereg->def->enum_hash) << ":00000000" << std::endl;
			for (int i = 0; i < ereg->def->values_count; i++) {
				out_file << std::format("{:08x}", ereg->def->values[i].hash) << ":00000000" << std::endl;
			}
			ereg = ereg->next;
		}

		std::ofstream out_file2("wordlist.txt", std::ios::trunc);

		if (!out_file2.is_open()) {
			printf("Failed to open export2 file!\n");
			return;
		}

		for (auto& hash : stringhash_library::hashes) {
			out_file2 << hash.second << std::endl;
		}
	}
};
