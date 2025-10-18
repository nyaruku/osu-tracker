#pragma once
#include <string>
#include <cstdint>
#include "console.h"
#include <crow/mustache.h>

namespace config {
	enum gameMode {
		osu = 0
		,taiko = 1
		,fruits = 2
		,mania = 3
	};

	enum server {
		bancho = 0
		,titanic = 1
	};

	struct application {
		int osuId = 0;
		int clientId = 0;
		std::string clientSecret;
		int apiInterval = 7000;
		config::gameMode gameMode = config::gameMode::fruits;
		config::server server = config::server::titanic;

		std::vector<std::tuple<std::string, std::string>> toArray() const {
			return {
				{"osuId", std::to_string(osuId)}
				,{ "clientId", std::to_string(clientId)}
				,{ "clientSecret", clientSecret}
				,{ "apiInterval", std::to_string(apiInterval)}
				,{ "gameMode", std::to_string(gameMode)}
				,{ "server", std::to_string(server)}
			};
		}
	};

	struct user {
		std::string username;
		std::string avatar;

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
		bool single;
		bool positive;
		bool display;
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

	namespace data {
		std::vector<dataEntry> arr {
			{"level",		"Level",			1, "", "",	"", dataType::d_float,		formatType::f_decimal,	false, true, true,	BANCHO | TITANIC}
			,{"rankedScore","Ranked Score",		2, "", "",	"", dataType::d_longLong,	formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"totalScore",	"Total Score",		3, "", "",	"", dataType::d_longLong,	formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"ppRank",		"PP Rank",			4, "", "",	"", dataType::d_int,		formatType::f_rank,		false, true, true,	BANCHO | TITANIC}
			,{"pp",			"PP",				5, "", "",	"", dataType::d_float,		formatType::f_decimal,	false, true, true,	BANCHO | TITANIC}
			,{"ppv1",		"PPv1",				6, "", "",	"", dataType::d_float,		formatType::f_decimal,	false, true, true,	TITANIC}
			,{"acc",		"Accuracy",			7, "", "",	"", dataType::d_float,		formatType::f_percent,	false, true, true,	BANCHO | TITANIC}
			,{"playtime",	"Play Time",		8, "", "",	"", dataType::d_longLong,	formatType::f_time,		false, true, true,	BANCHO | TITANIC}
			,{"playcount",	"Play Count",		9, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"totalHits",	"Total Hits",		10, "", "",	"", dataType::d_longLong,	formatType::f_int,		false, true, true,	TITANIC}
			,{"silverSS",	"Rank SSH",			11, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"goldSS",		"Rank SS",			12, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"silverS",	"Rank SH",			13, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"goldS",		"Rank S",			14, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"a",			"Rank A",			15, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"b",			"Rank B",			16, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"c",			"Rank C",			17, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"d",			"Rank D",			18, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"totalSS",	"Total SS",			19, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"totalS",		"Total S",			20, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"clears",		"Profile Clears",	21, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"totalClears","Total Clears",		22, "", "",	"", dataType::d_int,		formatType::f_int,		false, true, true,	BANCHO | TITANIC}
			,{"completion",	"Completion%",		23, "", "",	"", dataType::d_float,		formatType::f_percent,	false, true, true,	BANCHO | TITANIC}
			,{"scoreRank",	"Score Rank",		24, "", "",	"", dataType::d_int,		formatType::f_rank,		false, true, true,	BANCHO}
			,{"targetRank",	"Target Rank",		25, "", "",	"", dataType::d_int,		formatType::f_rank,		true,  true, true,	BANCHO}
			,{"targetUser",	"Target Player",	26, "", "",	"", dataType::d_string,		formatType::f_string,	true,  true, true,	BANCHO}
			,{"targetScore","Target Score",		27, "", "",	"", dataType::d_longLong,	formatType::f_int,		true,  true, true,	BANCHO}
		};

		static inline std::vector<dataEntry> arrFormatted;

		enum key {
			level = 0,
			rankedScore = 1,
			totalScore = 2,
			ppRank = 3,
			pp = 4,
			ppv1 = 5,
			acc = 6,
			playtime = 7,
			playcount = 8,
			totalHits = 9,
			silverSS = 10,
			goldSS = 11,
			silverS = 12,
			goldS = 13,
			a = 14,
			b = 15,
			c = 16,
			d = 17,
			totalSS = 18,
			totalS = 19,
			clears = 20,
			totalClears = 21,
			completion = 22,
			scoreRank = 23,
			targetRank = 24,
			targetUser = 25,
			targetScore = 26
		};

		inline constexpr int getIndex(key arrKey) {
			return static_cast<int>(arrKey);
		}
	};

	static void writeStats() {
		for (auto const& dir_entry : std::filesystem::directory_iterator{ "tracker_txt/template" }) {
			std::string content;
			if (std::ifstream file{ dir_entry.path() }; file.is_open()) {
				for (std::string line; std::getline(file, line); ) {
					for (size_t i = 0; i < config::data::arr.size(); ++i) {
						// raw
						line = ext::replace(line, "{{" + config::data::arr[i].key + "_init_raw}}", config::data::arr[i].init);
						line = ext::replace(line, "{{" + config::data::arr[i].key + "_change_raw}}", config::data::arr[i].change);
						line = ext::replace(line, "{{" + config::data::arr[i].key + "_current_raw}}", config::data::arr[i].current);

						// formatted
						line = ext::replace(line, "{{" + config::data::arr[i].key + "_init}}", config::data::arrFormatted[i].init);
			            line = ext::replace(line, "{{" + config::data::arr[i].key + "_change}}", config::data::arrFormatted[i].change);
			            line = ext::replace(line, "{{" + config::data::arr[i].key + "_current}}", config::data::arrFormatted[i].current);
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

	void writeConfig() {
		// ordered_json bc it sorts alphabetically by default
		nlohmann::ordered_json config;
		config::application application;
		for (const auto& [key, value] : application.toArray()) {
			if (std::empty(value)) {
				config["Main"][key] = nullptr;
				continue;
			}
			config["Main"][key] = value;
		}
		for (const config::dataEntry _vecData : config::data::arr) {
			bool display = (_vecData.dataSettings & DISPLAY) != 0;

			config["Display"][_vecData.key]["visible"] = _vecData.display;
			config["Display"][_vecData.key]["sort"] = _vecData.sort;
		}
		std::ofstream file;
		file.open("config.json");
		file << config.dump(4);
		file.close();
	}

	void readConfig() {
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
	
	void rmConfig() {
		std::filesystem::remove("config.json");
	}

	void createTemplateExample() {
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
