#pragma once
#include "STU.h"
// PARALIRA: � was put next to 
// "Statescript.h", now it is save here
#include "Statescript.h"
#include <fstream>
#include <iostream>
#include "stringhash_library.h"

class hash_exporter {
public:
	static inline void export_hashes() {
		std::ofstream out_file("hashcat.txt", std::ios::trunc);
		
		if (!out_file.is_open()) {
			printf("Failed to open export file!\n");
			return;
		}
		out_file << std::hex;

		STURegistry* reg = GetSTURegistry();
		while (reg) {
			out_file << fmt::format("{:08x}", reg->info->name_hash) << ":00000000" << std::endl;
			for (int i = 0; i < reg->info->argument_count; i++) {
				out_file << fmt::format("{:08x}", reg->info->arguments[i].name_hash) << ":00000000" << std::endl;
			}
			reg = reg->next;
		}

		STU_EnumRegistry* ereg = STUEnumRegistry();
		while (ereg) {
			out_file << fmt::format("{:08x}", ereg->def->enum_hash) << ":00000000" << std::endl;
			for (int i = 0; i < ereg->def->values_count; i++) {
				out_file << fmt::format("{:08x}", ereg->def->values[i].hash) << ":00000000" << std::endl;
			}
			ereg = ereg->next;
		}

		std::ofstream out_file2("wordlist.txt", std::ios::trunc);

		if (!out_file2.is_open()) {
			printf("Failed to open export2 file!\n");
			return;
		}

		for (auto& hash : allmighty_hash_lib::hashes) {
			out_file2 << hash.second << std::endl;
		}
	}
};