#pragma once
#include <string>
#include <cstdint>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <vector>
#include <Core/Console.h>
#include <Core/Helpers.h>
#include <crow/mustache.h>

namespace Core::Config {

	// FNV-1a, used by AppConfig and data::getIndex for O(1) key dispatch.
	static constexpr uint32_t hash(std::string_view str) {
		uint32_t h = 2166136261u;
		for (char c : str) {
			h ^= static_cast<uint32_t>(c);
			h *= 16777619u;
		}
		return h;
	}

	enum gameMode  { osu = 0, taiko = 1, fruits = 2, mania = 3 };
	enum server    { bancho = 0, titanic = 1 };
	enum dataType  { d_string = 0, d_int = 1, d_float = 2, d_longLong = 3 };
	enum formatType{ f_string = 0, f_int = 1, f_decimal = 2, f_rank = 3, f_time = 4, f_percent = 5 };

	struct AppConfig {
		int        banchoId     = 0;
		int        titanicId    = 0;
		int        clientId     = 0;
		std::string clientSecret;
		int        apiInterval  = 7000;
		gameMode   gameMode     = gameMode::osu;
		server     server       = server::bancho;

		// Returns the user ID for the currently active server.
		int activeId() const {
			return (server == titanic) ? titanicId : banchoId;
		}

		void set(std::string_view key, const std::string& value) {
			switch (hash(key)) {
				case hash("osuId"):        banchoId     = std::stoi(value);                                return; // backward
				case hash("banchoId"):     banchoId     = std::stoi(value);                                return;
				case hash("titanicId"):    titanicId    = std::stoi(value);                                return;
				case hash("clientId"):     clientId     = std::stoi(value);                                return;
				case hash("clientSecret"): clientSecret = value;                                           return;
				case hash("apiInterval"):  apiInterval  = std::stoi(value);                                return;
				case hash("gameMode"):     gameMode     = static_cast<Config::gameMode>(std::stoi(value)); return;
				case hash("server"):       server       = static_cast<Config::server>(std::stoi(value));   return;
			}
		}

		std::string get(std::string_view key) const {
			switch (hash(key)) {
				case hash("banchoId"):     return std::to_string(banchoId);
				case hash("titanicId"):    return std::to_string(titanicId);
				case hash("clientId"):     return std::to_string(clientId);
				case hash("clientSecret"): return clientSecret;
				case hash("apiInterval"):  return std::to_string(apiInterval);
				case hash("gameMode"):     return std::to_string(static_cast<int>(gameMode));
				case hash("server"):       return std::to_string(static_cast<int>(server));
			}
			return "";
		}

		std::vector<std::tuple<std::string, std::string>> toArray() const {
			return {
				{"banchoId",     std::to_string(banchoId)},
				{"titanicId",    std::to_string(titanicId)},
				{"clientId",     std::to_string(clientId)},
				{"clientSecret", clientSecret},
				{"apiInterval",  std::to_string(apiInterval)},
				{"gameMode",     std::to_string(static_cast<int>(gameMode))},
				{"server",       std::to_string(static_cast<int>(server))},
			};
		}
	};

	struct UserConfig {
		std::string username;
		std::string avatarUrl;
		std::vector<unsigned char> avatarBytes;
		std::mutex                 avatarMutex;
		std::atomic<bool>          avatarDirty{ false };
	};

	struct dataEntry {
		std::string key;
		std::string name;
		int         sort;
		std::string init;
		std::string current;
		std::string change;
		dataType    dataType;
		formatType  formatType;
		bool        single;
		bool        positive;
		bool        display;
		bool        banchoSupport;
		bool        titanicSupport;

		std::vector<std::tuple<std::string, std::string>> toArray() const {
			return {
				{"key",            key},
				{"name",           name},
				{"sort",           std::to_string(sort)},
				{"init",           init},
				{"current",        current},
				{"change",         change},
				{"dataType",       std::to_string(static_cast<int>(dataType))},
				{"formatType",     std::to_string(static_cast<int>(formatType))},
				{"positive",       Helpers::bool2str(positive)},
				{"display",        Helpers::bool2str(display)},
				{"single",         Helpers::bool2str(single)},
				{"banchoSupport",  Helpers::bool2str(banchoSupport)},
				{"titanicSupport", Helpers::bool2str(titanicSupport)},
			};
		}
	};

	inline AppConfig  application;
	inline UserConfig user;

	namespace data {

		inline std::vector<dataEntry> arr {
			// key                  name                      sort  init cur chg  dataType              formatType             single  pos   disp   bancho titanic
			{"level",               "Level",                     1,  "","","",  dataType::d_float,    formatType::f_decimal,  false, true,  true,  true,  true }, //  0
			{"scoreRank",           "Score Rank",                2,  "","","",  dataType::d_int,      formatType::f_rank,     false, true,  true,  true,  true }, //  1
			{"ppRank",              "PP Rank",                   3,  "","","",  dataType::d_int,      formatType::f_rank,     false, true,  true,  true,  true }, //  2
			{"ppv1Rank",            "PPv1 Rank",                 4,  "","","",  dataType::d_int,      formatType::f_rank,     false, true,  true,  false, true }, //  3
			{"countryRank",         "Country Rank",              5,  "","","",  dataType::d_int,      formatType::f_rank,     false, true,  true,  false, true }, //  4

			{"pp",                  "PP",                        6,  "","","",  dataType::d_float,    formatType::f_decimal,  false, true,  true,  true,  true }, //  5
			{"ppv1",                "PPv1",                      7,  "","","",  dataType::d_float,    formatType::f_decimal,  false, true,  true,  false, true }, //  6
			{"acc",                 "Accuracy",                  8,  "","","",  dataType::d_float,    formatType::f_percent,  false, true,  true,  true,  true }, //  7

			{"playtime",            "Play Time",                 9,  "","","",  dataType::d_longLong, formatType::f_time,     false, true,  true,  true,  true }, //  8
			{"playcount",           "Play Count",               10,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, //  9
			{"maxCombo",            "Max Combo",                11,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  false, true }, // 10
			{"totalHits",           "Total Hits",               12,  "","","",  dataType::d_longLong, formatType::f_int,      false, true,  true,  true,  true }, // 11
			{"replayViews",         "Replay Views",             13,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  false, true }, // 12

			{"rankedScore",         "Ranked Score",             14,  "","","",  dataType::d_longLong, formatType::f_int,      false, true,  true,  true,  true }, // 13
			{"totalScore",          "Total Score",              15,  "","","",  dataType::d_longLong, formatType::f_int,      false, true,  true,  true,  true }, // 14

			{"clears",              "Profile Clears",           16,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  false },
			{"totalClears",         "Total Clears",             17,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 16

			{"silverSS",            "Rank SSH",                 18,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 17
			{"goldSS",              "Rank SS",                  19,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 18
			{"silverS",             "Rank SH",                  20,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 19
			{"goldS",               "Rank S",                   21,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 20
			{"a",                   "Rank A",                   22,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 21
			{"b",                   "Rank B",                   23,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 22
			{"c",                   "Rank C",                   24,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 23
			{"d",                   "Rank D",                   25,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 24
			{"totalSS",             "Total SS",                 26,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 25
			{"totalS",              "Total S",                  27,  "","","",  dataType::d_int,      formatType::f_int,      false, true,  true,  true,  true }, // 26

			{"targetRank",          "Target Rank",              28,  "","","",  dataType::d_int,      formatType::f_rank,     true,  true,  true,  true,  false}, // 27
			{"targetUser",          "Target Player",            29,  "","","",  dataType::d_string,   formatType::f_string,   true,  true,  true,  true,  false}, // 28
			{"targetScore",         "Target Score",             30,  "","","",  dataType::d_longLong, formatType::f_int,      true,  true,  true,  true,  false}, // 29
			{"completion",          "Completion%",              31,  "","","",  dataType::d_float,    formatType::f_percent,  false, true,  true,  true,  true }, // 30
		};

		// Formatted copy of arr - written by fetchThread, read by web routes and imgui.
		inline std::vector<dataEntry> arrFormatted;

		// Protects arr and arrFormatted against concurrent read/write.
		// fetchThread holds unique_lock while writing; all readers hold shared_lock.
		inline std::shared_mutex dataMutex;

		struct EntryDefaults { bool display; int sort; };
		inline std::vector<EntryDefaults> defaults;

		// Call once before any config::read() to snapshot compiled-in defaults.
		inline void initDefaults() {
			if (!defaults.empty()) return;
			for (const auto& e : arr)
				defaults.push_back({ e.display, e.sort });
		}

		// Restore display/sort on every entry to the compiled-in defaults.
		inline void resetDefaults() {
			for (size_t i = 0; i < arr.size() && i < defaults.size(); ++i) {
				arr[i].display = defaults[i].display;
				arr[i].sort    = defaults[i].sort;
			}
		}

		inline constexpr int getIndex(std::string_view key) {
			switch (hash(key)) {
				case hash("level"):              return  0;
				case hash("scoreRank"):          return  1;
				case hash("ppRank"):             return  2;
				case hash("ppv1Rank"):           return  3;
				case hash("countryRank"):        return  4;
				case hash("pp"):                 return  5;
				case hash("ppv1"):               return  6;
				case hash("acc"):                return  7;
				case hash("playtime"):           return  8;
				case hash("playcount"):          return  9;
				case hash("maxCombo"):           return 10;
				case hash("totalHits"):          return 11;
				case hash("replayViews"):        return 12;
				case hash("rankedScore"):        return 13;
				case hash("totalScore"):         return 14;
				case hash("clears"):             return 15;
				case hash("totalClears"):        return 16;
				case hash("silverSS"):           return 17;
				case hash("goldSS"):             return 18;
				case hash("silverS"):            return 19;
				case hash("goldS"):              return 20;
				case hash("a"):                  return 21;
				case hash("b"):                  return 22;
				case hash("c"):                  return 23;
				case hash("d"):                  return 24;
				case hash("totalSS"):            return 25;
				case hash("totalS"):             return 26;
				case hash("targetRank"):         return 27;
				case hash("targetUser"):         return 28;
				case hash("targetScore"):        return 29;
				case hash("completion"):         return 30;
			}
			return -1;
		}

	}

	inline void resetToDefaults() {
		application = AppConfig{};
		data::resetDefaults();
	}

	inline void write() {
		nlohmann::ordered_json j;
		for (const auto& [key, value] : application.toArray()) {
			j["Main"][key] = value.empty() ? nlohmann::ordered_json(nullptr) : nlohmann::ordered_json(value);
		}
		for (const dataEntry& e : data::arr) {
			j["Display"][e.key]["visible"] = e.display;
			j["Display"][e.key]["sort"]    = e.sort;
		}
		if (std::ofstream file{ "config.json" }; file.is_open())
			file << j.dump(4);
	}

	inline void read() {
		try {
			if (std::ifstream file{ "config.json" }; file.is_open()) {
				nlohmann::ordered_json j = nlohmann::ordered_json::parse(file);
				if (j.contains("Main") && j["Main"].is_object()) {
					for (auto& [key, val] : j["Main"].items()) {
						application.set(key, val.is_null() ? "" : val.is_string() ? val.get<std::string>() : val.dump());
					}
				}
				if (j.contains("Display") && j["Display"].is_object()) {
					for (auto& [key, val] : j["Display"].items()) {
						int idx = data::getIndex(key);
						if (idx < 0 || !val.is_object()) continue;
						if (val.contains("visible") && val["visible"].is_boolean())
							data::arr[idx].display = val["visible"].get<bool>();
						if (val.contains("sort") && val["sort"].is_number())
							data::arr[idx].sort = val["sort"].get<int>();
					}
				}
			} else {
				Console::writeLog("Error reading config.json", true, 255, 0, 0);
			}
		}
		catch (const nlohmann::json::exception& e) { Console::writeLog("config::read() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
		catch (const std::exception& e)            { Console::writeLog("config::read() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
	}

	inline void remove() {
		std::filesystem::remove("config.json");
	}

	inline void writeStats() {
		for (const auto& entry : std::filesystem::directory_iterator{ "Txt/template" }) {
			std::string content;
			if (std::ifstream file{ entry.path() }; file.is_open()) {
				for (std::string line; std::getline(file, line); ) {
					for (size_t i = 0; i < data::arr.size(); ++i) {
						const auto& raw = data::arr[i];
						const auto& fmt = data::arrFormatted[i];
						const std::string& k = raw.key;
						line = Helpers::replace(line, "{{" + k + "_init_raw}}",    raw.init);
						line = Helpers::replace(line, "{{" + k + "_change_raw}}", raw.change);
						line = Helpers::replace(line, "{{" + k + "_current_raw}}", raw.current);
						line = Helpers::replace(line, "{{" + k + "_init}}",        fmt.init);
						line = Helpers::replace(line, "{{" + k + "_change}}",      fmt.change);
						line = Helpers::replace(line, "{{" + k + "_current}}",     fmt.current);
					}
					content += line + '\n';
				}
			} else {
				Console::writeLog("Error reading " + entry.path().string(), true, 255, 0, 0);
				continue;
			}
			if (std::ofstream out{ "Txt/" + entry.path().filename().string() }; out.is_open())
				out << content;
		}
	}

	inline void createTemplateExample() {
		std::string content;
		for (const dataEntry& e : data::arr) {
			content += e.key + " (raw):\n";
			content += "  init:    {{" + e.key + "_init_raw}}\n";
			content += "  change:  {{" + e.key + "_change_raw}}\n";
			content += "  current: {{" + e.key + "_current_raw}}\n";
			content += e.key + " (formatted):\n";
			content += "  init:    {{" + e.key + "_init}}\n";
			content += "  change:  {{" + e.key + "_change}}\n";
			content += "  current: {{" + e.key + "_current}}\n\n";
		}
		if (std::ofstream file{ "Txt/template/example.txt" }; file.is_open())
			file << content;
	}

	#ifdef DEBUG_BUILD
	inline void readEnv(const std::string& path = "debug.env") {
		if (std::ifstream file{ path }; file.is_open()) {
			Console::writeLog("[DEBUG] Loading " + path + "...", true, 255, 165, 0);
			for (std::string line; std::getline(file, line); ) {
				if (line.empty() || line[0] == '#') continue;
				auto [key, value] = Helpers::split2tuple(line, '=');
				if (key.empty()) continue;
				application.set(key, value);
				const std::string masked = (key == "clientSecret") ? std::string(value.size(), '*') : value;
				Console::writeLog("[DEBUG] env  " + key + " = " + masked, true, 255, 165, 0);
			}
			Console::writeLog("[DEBUG] debug.env applied.", true, 255, 165, 0);
		} else {
			Console::writeLog("[DEBUG] debug.env not found - using config.json values.", true, 255, 165, 0);
		}
	}
	#endif

}