#include "filetype_library.h"
#include <imgui.h>
#include "game.h"
#include "window_manager/window_manager.h"
#include <fast-cpp-csv-parser/csv.h>
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

const char* default_filetypes = R"{--(
{
    "002": "Map Data",
    "003": "STUEntityDefinition",
    "004": "Texture, (sometimes w/ 04D)",
    "006": "Animation",
    "007": "teModelChunk_Skeleton (mskl)",
    "008": "teMaterial",
    "00C": "Model",
    "00D": "Effect, GFX",
    "015": "STUAnimCategory",
    "018": "STUAnimBoneWeightMask",
    "01A": "STUModelLook",
    "01B": "STUStatescriptGraph",
    "020": "STUAnimBlendTree",
    "021": "STUAnimBlendTreeSet (template?)",
    "024": "Statescript Priority (m_priority)",
    "2B": "STUSoundEvent",
    "02C": "STUSound",
    "02D": "STUSoundSwitchGroup",
    "02E": "STUSoundSwitch",
    "02F": "STUSoundParameter",
    "030": "STUSoundStateGroup",
    "031": "STUSoundState",
    "33": "STUSoundBankData",
    "039": "STUMapCatalog bind 002->09F",
    "03F": "SoundWEMFile",
    "043": "SoundBank",
    "045": "STUUXIDLookup",
    "049": "STUContactSet",
    "04A": "Effect Environment FX?",
    "04B": "JSON ca_bundle.txt.signed",
    "04C": "Sound something? Ref'd by 02C. References 02F and 013",
    "04D": "TexturePayload Has a corresponding 004 file",
    "04E": "Sound something? Ref'd by 04C. References 02E and 013",
    "050": "Font Wrapped TTF / OTF",
    "051": "STUFontFamily",
    "53": "STUAura",
    "054": "STUGenericSettings_Base",
    "055": "STUSoundSpace",
    "058": "STUProgressionUnlocks",
    "05A": "STUUXScreen",
    "05E": "STUUXResourceDictionary",
    "05F": "VoiceSet STUVoiceSet",
    "062": "Stat STUStat",
    "063": "STUCatalog",
    "065": "STUMaterialEffect",
    "066": "added 1.48",
    "068": "STUAchievement",
    "070": "STUVoiceLineSet",
    "071": "Subtitle STU",
    "072": "STULocaleSettings",
    "075": "STUHero",
    "078": "STUVoiceStimulus, a voice sound 'trigger'",
    "079": "STUVoiceCategory",
    "07C": "UXDisplayString",
    "07F": "Chat replacement",
    "085": "Shader Group == 088, pixel + vertex",
    "086": "Shader Instance",
    "087": "Shader Code",
    "088": "Shader Source == 085. dev src diff?",
    "08E": "Effect Unknown Effect",
    "08F": "Effect Animation Effect",
    "090": "STUResourceKey",
    "091": "added 1.48",
    "095": "STUInputPhysicalButton",
    "096": "System Mapping",
    "097": "STURaycastType",
    "098": "STURaycastReceiver",
    "09C": "PackageBundle, Binary stream, use package offsets to find files",
    "09D": "Announcer",
    "09E": "STULoadout (ability or weapon)",
    "09F": "STUMapHeader",
    "0A3": "STUSoundIntegrity",
    "0A5": "STUUnlock",
    "0A6": "STUSkinTheme",
    "0A8": "STUEffectLook",
    "0A9": "String, Unlocalized",
    "0AA": "STUMapFont",
    "0AC": "STUGamePadVibration, controller vibration curves",
    "0AD": "STUHeroWeapon",
    "0B2": "VoiceWEMFile",
    "0B3": "MaterialData",
    "0B6": "Movie, Bink Video",
    "0B7": "STUUXLogicalButton",
    "0BB": "Video Audio, WWise WEM",
    "0BC": "Map Chunk",
    "0BD": "Lighting Manifest",
    "0BE": "Lighting Chunk",
    "0BF": "STULineupPose",
    "0C0": "STUGameRuleset",
    "0C2": "Highlight Type",
    "0C5": "STUGameMode",
    "0C6": "STUGameRulesetSchema",
    "0C7": "Brawl",
    "0C9": "Sound something? Ref'd by 02C. Has arrays of 04E's",
    "0CB": "Shadow Data",
    "0CC": "Arcade (0EE) Descriptions",
    "0CE": "STUTeamColor",
    "0CF": "STULootBox",
    "0D0": "VoiceConversation, STUVoiceConversation",
    "0D5": "Game Tips",
    "0D6": "Something to do with Retribution/Storm Rising. References 0D7",
    "0D7": "Retribution/Storm Rising enemy definitions? References NPCs STUHero. Referenced by 0D6",
    "0D9": "Brawl Name",
    "0EB": "Report Response, The message when a player you reported is punished or wh<en you are punished",
    "0EC": "League Team, OWL team definition",
    "0EE": "Arcade, Arcade modes",
    "0F1": "Localized Texture, Localized to selected language. added 1.22",
    "114": "Graph Type (UX?), added 1.44",
    "116": "UX Stuff, added 1.44",
    "127": "material of some kind? added 1.48",
    "00E": "Light Data",
    "00F": "Shader Group Cache",
    "010": "Display String",
    "013": "STUDataFlow",
    "014": "STUAnimAlias",
    "025": "STUGameMessage",
    "02A": "STUSoundFilter",
    "034": "Asset Thumbnail removed",
    "036": "STUUXLink",
    "03E": "Light Debug Data removed",
    "04F": "1.22 removed",
    "06F": "STUVoiceLine",
    "076": "STUSkin",
    "077": "teAssetPackage",
    "09B": "Sound Binding",
    "0A1": "removed 1.7",
    "0AF": "Enum > String Mutator? - UI stuff, references bunch of types removed 1.44",
    "0B0": "STUUXViewModelSchema",
    "0C4": "Brawl Information removed 1.5",
    "0C8": "RankedSeason STURankedSeason removed 1.38",
    "1c": "CVD"
}
){--";

namespace filetype_library {
	std::map<int, char*> library;
	bool will_save = false;

	/*                class trim_policy = trim_chars<' ', '\t'>,
                class quote_policy = no_quote_escape<','>,
                class overflow_policy = throw_on_overflow,
*/
	typedef typename io::CSVReader<4, io::trim_chars<' ', '\t'>, io::no_quote_escape<','>, io::throw_on_overflow, io::single_and_empty_line_comment<';'>> csv;
	std::unique_ptr<csv> names_file;
	//file_type, file_id
	std::map<std::pair<int, int>, std::string> file_names{};

	const char* filename = "filetypes.json";
	void json_parse(nlohmann::json json) {
		if (!json.is_object()) {
			return;
		}

		for (auto it = json.begin(); it != json.end(); ++it) {
			int type = strtoll(it.key().c_str(), nullptr, 16);
			if (type == 0) {
				continue;
			}
			auto value = it.value().get<std::string>();
			char* pvalue = new char[256];
			strcpy(pvalue, value.c_str());

			library.emplace(type, pvalue);
		}
	}

	void init() {
		try {
			json_parse(nlohmann::json::parse(default_filetypes));
			nlohmann::json json;
			{
				std::ifstream input_file(filename);
				if (!input_file.is_open()) {
					printf("Failed to open input filetype file.");
					return;
				}
				json = nlohmann::json::parse(input_file);
			}
			json_parse(json);

			will_save = true;
		}
		catch (nlohmann::json::exception ex) {
			printf("Failed to open filetypes.json. Saving is disabled. JSON Error: %s\n", ex.what());
		}
		catch (std::exception ex) {
			printf("Failed to open filetypes.json. Saving is disabled. Error: %s\n", ex.what());
		}
		catch (...) {
			printf("Failed to open filetypes.json. Saving is disabled.\n");
		}
	}

	void save_filetypes() {
		if (!will_save) {
			imgui_helpers::messageBox("Saving filetypes is disabled.", "Hashlib");
			return;
		}
		try {
			nlohmann::json result{};
			for (auto& hash : library) {
				char buf[16];
				sprintf_s(buf, "%x", hash.first);
				result.emplace(buf, hash.second);
			}
			std::ofstream fstream(filename, std::ios::out | std::ios::trunc);
			if (!fstream.is_open()) {
				imgui_helpers::messageBox("Filestream open error", "Hashlib");
				return;
			}
			fstream << result.dump(4);
			fstream.flush();
		}
		catch (nlohmann::json::exception ex) {
			printf("Failed to save filetypes.json. Saving is disabled. JSON Error: %s\n", ex.what());
		}
		catch (std::exception ex) {
			printf("Failed to save filetypes.json. Saving is disabled. Error: %s\n", ex.what());
		}
		catch (...) {
			printf("Failed to save filetypes.json. Saving is disabled.\n");
		}
	}

	void display_filetype(__int64 filetype) {
		ImGui::Text("%p", filetype);
		
		ImGui::SameLine();
		char buf[64];
		sprintf_s(buf, "Copy##%p", filetype);
		if (ImGui::Button(buf)) {
			imgui_helpers::openCopyWindow(filetype);
		}
		memset(buf, 0, sizeof(buf));
		ImGui::SameLine();
		sprintf(buf, "Copy Search##%p", filetype);
		if (ImGui::Button(buf)) {
			imgui_helpers::openCopyWindow(filetype);
		}

		int type = bitswap(16 * (filetype & 0xFFFF000000000000uLL)) + 1;
		int fileid = filetype & 0xFFFF;

		auto filename = file_names.find(std::pair<int, int>{type, fileid});
		if (filename != file_names.end()) {
			//ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 0, 255, 255)));
			ImGui::Text("FILENAME -> %s", filename->second.c_str());
			ImGui::PopStyleColor();
		}

		auto found = library.find(type);
		if (found != library.end()) {
			//ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 255, 0, 255)));
			ImGui::Text("-> %s", found->second);
			ImGui::PopStyleColor();
		}
	}
}