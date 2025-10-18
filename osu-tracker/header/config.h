#pragma once
#include <string>
#include <cstdint>
#include "console.h"
#include <crow/mustache.h>

#define FOREACH_DATA_KEY(F) \
	F(level) \
	F(rankedScore) \
	F(totalScore) \
	F(ppRank) \
	F(pp) \
	F(ppv1) \
	F(acc) \
	F(playtime) \
	F(playcount) \
	F(totalHits) \
	F(silverSS) \
	F(goldSS) \
	F(silverS) \
	F(goldS) \
	F(a) \
	F(b) \
	F(c) \
	F(d) \
	F(totalSS) \
	F(totalS) \
	F(clears) \
	F(totalClears) \
	F(completion) \
	F(scoreRank) \
	F(targetRank) \
	F(targetUser) \
	F(targetScore)

constexpr bool str_eq(const char* a, const char* b) {
	while (*a && *b) {
		if (*a != *b) return false;
		++a; ++b;
	}
	return *a == *b;
}

#define GEN_CASE(name, idx) if (str_eq(key, #name)) return idx;

namespace config {
	enum class gameMode {
		osu = 0
		,taiko = 1
		,fruits = 2
		,mania = 3
	};

	enum class server {
		bancho = 0
		,titanic = 1
	};

	struct application {
		// definition and initialization
		#define applicationFields(_field) \
			_field(int, osuId, 0) \
			_field(int, clientId, 0) \
			_field(std::string, clientSecret, "") \
			_field(int, apiInterval, 7000) \
			_field(config::gameMode, gameMode, config::gameMode::osu) \
			_field(config::server, server, config::server::bancho)
	};

	struct user {
		std::string username = "";
		std::string avatar = "";

		std::vector<std::tuple<std::string, std::string>> toArray() const {
			return {
				{"username", username}
				,{ "avatar", avatar}
			};
		}
	};

	enum dataType {
		d_string = 0
		,d_int = 1
		,d_float = 2
		,d_longLong = 3
	};

	enum formatType {
		f_string = 0
		,f_int = 1
		,f_decimal = 2
		,f_rank = 3
		,f_time = 4
		,f_percent = 5
	};

	enum dataSettings : uint32_t {
		POSITIVE = 1 << 0
		,DISPLAY = 1 << 1
		,SINGLE = 1 << 2
	};

	enum serverSettings : uint32_t {
		BANCHO = 1 << 0
		,TITANIC = 1 << 1
	};

	struct dataEntry {
		const std::string key;
		const std::string name;
		int sort;
		std::string init;
		std::string current;
		std::string change;
		dataType dataType;
		formatType formatType;
		u_int32_t dataSettings;
		u_int32_t serverSettings;

		std::vector<std::tuple<std::string, std::string>> toArray() const {
			return {
				{"key", key}
				,{"name", name}
				,{"sort", std::to_string(sort)}
				,{"init", init}
				,{"current", current}
				,{"change", change}
				,{"dataType", std::to_string(static_cast<int>(dataType))}
				,{"formatType", std::to_string(static_cast<int>(formatType))}
				,{"dataSettings", std::to_string(dataSettings)}
				,{"serverSettings", std::to_string(serverSettings)}
			};
		}
	};
// positive display single
	namespace data {
		static inline std::vector<dataEntry> arr {
			{"level",		"Level",			1, "", "",	"", dataType::d_float,		formatType::f_decimal,	POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"rankedScore","Ranked Score",		2, "", "",	"", dataType::d_longLong,	formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"totalScore",	"Total Score",		3, "", "",	"", dataType::d_longLong,	formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"ppRank",		"PP Rank",			4, "", "",	"", dataType::d_int,		formatType::f_rank,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"pp",			"PP",				5, "", "",	"", dataType::d_float,		formatType::f_decimal,	POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"ppv1",		"PPv1",				6, "", "",	"", dataType::d_float,		formatType::f_decimal,	POSITIVE | DISPLAY,	TITANIC}
			,{"acc",		"Accuracy",			7, "", "",	"", dataType::d_float,		formatType::f_percent,	POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"playtime",	"Play Time",		8, "", "",	"", dataType::d_longLong,	formatType::f_time,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"playcount",	"Play Count",		9, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"totalHits",	"Total Hits",		10, "", "",	"", dataType::d_longLong,	formatType::f_int,		POSITIVE | DISPLAY,	TITANIC}

			,{"silverSS",	"Rank SSH",			11, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"goldSS",		"Rank SS",			12, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"silverS",	"Rank SH",			13, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"goldS",		"Rank S",			14, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"a",			"Rank A",			15, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"b",			"Rank B",			16, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"c",			"Rank C",			17, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"d",			"Rank D",			18, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"totalSS",	"Total SS",			19, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"totalS",		"Total S",			20, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}

			,{"clears",		"Profile Clears",	21, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"totalClears","Total Clears",		22, "", "",	"", dataType::d_int,		formatType::f_int,		POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"completion",	"Completion%",		23, "", "",	"", dataType::d_float,		formatType::f_percent,	POSITIVE | DISPLAY,	BANCHO | TITANIC}
			,{"scoreRank",	"Score Rank",		24, "", "",	"", dataType::d_int,		formatType::f_rank,		POSITIVE | DISPLAY,	BANCHO}

			,{"targetRank",	"Target Rank",		25, "", "",	"", dataType::d_int,		formatType::f_rank,		DISPLAY | SINGLE,	BANCHO}
			,{"targetUser",	"Target Player",	26, "", "",	"", dataType::d_string,		formatType::f_string,	DISPLAY | SINGLE,	BANCHO}
			,{"targetScore","Target Score",		27, "", "",	"", dataType::d_longLong,	formatType::f_int,		DISPLAY | SINGLE,	BANCHO}
		};

		static inline std::vector<dataEntry> arrFormatted;

		static constexpr int getIndex(const char* key) {
		#define INDEX_CASE(name) GEN_CASE(name, __COUNTER__)
			FOREACH_DATA_KEY(INDEX_CASE)
				return -1;
		}
	};

	static void writeStats() {
		for (auto const& dir_entry : std::filesystem::directory_iterator{ "tracker_txt/template" }) {
			std::string content;
			if (std::ifstream file{ dir_entry.path() }; file.is_open()) {
				for (std::string line; std::getline(file, line); ) {
					for (dataEntry row : config::data::arr) {
						// raw
						line = ext::replace(line, "{{" + row.key + "_init_raw}}", row.init);
						line = ext::replace(line, "{{" + row.key + "_change_raw}}", row.change);
						line = ext::replace(line, "{{" + row.key + "_current_raw}}", row.current);

						// formatted
						line = ext::replace(
							line,
							"{{" + row.key + "_init}}",
							config::data::arrFormatted[config::data::getIndex(row.key.c_str())].init
						);
						
			            line = ext::replace(
				            line,
				            "{{" + row.key + "_change}}",
				            config::data::arrFormatted[config::data::getIndex(row.key.c_str())].change
			            );
						
			            line = ext::replace(
							line,
							"{{" + row.key + "_current}}",
							config::data::arrFormatted[config::data::getIndex(row.key.c_str())].current
			            );
					}
					content += line + "\n";
				}
			}
			else {
				console::writeLog("Error reading " + dir_entry.path().string(), true, 255, 0, 0);
			}
			std::ofstream file;
			file.open("tracker_txt/" + dir_entry.path().filename().string());
			file << content;
			file.close();
			file.clear();
		}
	}

	static void writeConfig() {
		// ordered_json bc it sorts alphabetically by default
		nlohmann::ordered_json config;
		for (const auto& [key, value] : config::application::instance().toArray()) {
			if (value == "") {
				config["Main"][key] = nullptr;
				continue;
			}
			config["Main"][key] = value;
		}
		for (const config::dataEntry _vecData : config::data::arr) {
			config["Display"][_vecData.key]["visible"] = _vecData.display;
			config["Display"][_vecData.key]["sort"] = _vecData.sort;
		}
		std::ofstream file;
		file.open("config.json");
		file << config.dump(4);
		file.close();
	}

	static void readConfig() {
		if (std::ifstream file{ "config.json" }; file.is_open()) {
			nlohmann::ordered_json config = nlohmann::ordered_json::parse(file);
			for (auto& main : config["Main"].items()) {
				if (main.value().is_null()) {
					config::application::set(main.key(), "");
				}
				else if (main.value().is_string()) {
					config::application::set(main.key(), main.value().get<std::string>());
				}
				else {
					// fallback: dump to string
					config::application::set(main.key(), main.value().dump());
				}
			}
			for (auto& display : config["Display"].items()) {
				config::data::arr[config::data::getIndex(display.key().c_str())].display = display.value()["visible"].get<bool>();
				config::data::arr[config::data::getIndex(display.key().c_str())].sort = display.value()["sort"].get<int>();
			}
			file.close();
		}
		else {
			console::writeLog("Error reading config file", true, 255, 0, 0);
		}
	}
	
	static void rmConfig() {
		std::filesystem::remove("config.json");
	}

	static void createTemplateExample() {
		std::string content;
		for (size_t i = 0; i < config::data::arr.size(); i++) {
			content += config::data::arr[i].key + " (raw):\n";
	 		content += "init: {{" + config::data::arr[i].key + "_init_raw}}\n";
   			content += "change: {{" + config::data::arr[i].key + "_change_raw}}\n";
   			content += "current: {{" + config::data::arr[i].key + "_current_raw}}\n";
			content += config::data::arr[i].key + " (formatted):\n";
    		content += "init: {{" + config::data::arr[i].key + "_init}}\n";
    		content += "change: {{" + config::data::arr[i].key + "_change}}\n";
    		content += "current: {{" + config::data::arr[i].key + "_current}}\n";
			content += "\n";
		}
		std::ofstream file;
		file.open("tracker_txt/template/example.txt");
		file << content;
		file.close();
		file.clear();
	}
};
