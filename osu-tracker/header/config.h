#pragma once
#include <string>
#include <cstdint>
#include <string_view>
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

		static constexpr uint32_t hash(std::string_view str) {
			uint32_t h = 2166136261u;
			for (char c : str) {
				h ^= static_cast<uint32_t>(c);
				h *= 16777619u;
			}
			return h;
		}

		bool set(std::string_view key, const std::string& value) {
			switch (hash(key)) {
				case hash("osuId"): osuId = std::stoi(value); return true;
				case hash("clientId"): clientId = std::stoi(value); return true;
				case hash("clientSecret"): clientSecret = value; return true;
				case hash("apiInterval"): apiInterval = std::stoi(value); return true;
				case hash("gameMode"): gameMode = static_cast<config::gameMode>(std::stoi(value)); return true;
				case hash("server"): server = static_cast<config::server>(std::stoi(value)); return true;
				default: return false; // key not found
			}
		}

		std::vector<std::tuple<std::string, std::string>> toArray() const {
			return {
	            {"osuId", std::to_string(osuId)},
				{"clientId", std::to_string(clientId)},
				{"clientSecret", clientSecret},
				{"apiInterval", std::to_string(apiInterval)},
				{"gameMode", std::to_string(static_cast<int>(gameMode))},
				{"server", std::to_string(static_cast<int>(server))}
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
		u_int32_t serverSupport;

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
				,{"positive", ext::bool2str(positive)}
				,{"display", ext::bool2str(display)}
				,{"single", ext::bool2str(single)}
				,{"serverSettings", std::to_string(serverSupport)}
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

		constexpr std::uint32_t hash(std::string_view str) {
			std::uint32_t h = 2166136261u;
			for (char c : str) {
				h ^= static_cast<std::uint32_t>(c);
				h *= 16777619u;
			}
			return h;
		}

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

		// no need for default
		inline constexpr int getIndex(std::string_view key) {
			switch (hash(key)) {
				case hash("level"): return 0;
				case hash("rankedScore"): return 1;
				case hash("totalScore"): return 2;
				case hash("ppRank"): return 3;
				case hash("pp"): return 4;
				case hash("ppv1"): return 5;
				case hash("acc"): return 6;
				case hash("playtime"): return 7;
				case hash("playcount"): return 8;
				case hash("totalHits"): return 9;
				case hash("silverSS"): return 10;
				case hash("goldSS"): return 11;
				case hash("silverS"): return 12;
				case hash("goldS"): return 13;
				case hash("a"): return 14;
				case hash("b"): return 15;
				case hash("c"): return 16;
				case hash("d"): return 17;
				case hash("totalSS"): return 18;
				case hash("totalS"): return 19;
				case hash("clears"): return 20;
				case hash("totalClears"): return 21;
				case hash("completion"): return 22;
				case hash("scoreRank"): return 23;
				case hash("targetRank"): return 24;
				case hash("targetUser"): return 25;
				case hash("targetScore"): return 26;
			}
		}

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

	inline void writeConfig() {
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
			config["Display"][_vecData.key]["visible"] = _vecData.display;
			config["Display"][_vecData.key]["sort"] = _vecData.sort;
		}
		std::ofstream file;
		file.open("config.json");
		file << config.dump(4);
		file.close();
	}

	inline void readConfig() {
		if (std::ifstream file{ "config.json" }; file.is_open()) {
			nlohmann::ordered_json config = nlohmann::ordered_json::parse(file);
			config::application application;
			for (auto& main : config["Main"].items()) {
				if (main.value().is_null()) {
					application.set(main.key(), "");
				}
				else if (main.value().is_string()) {
					application.set(main.key(), main.value().get<std::string>());
				}
				else {
					// fallback: dump to string
					application.set(main.key(), main.value().dump());
				}
			}
			for (auto& display : config["Display"].items()) {
				config::data::arr[config::data::getIndex(display.key())].display = display.value()["visible"].get<bool>();
				config::data::arr[config::data::getIndex(display.key())].sort = display.value()["sort"].get<int>();
			}
			file.close();
		}
		else {
			console::writeLog("Error reading config file", true, 255, 0, 0);
		}
	}
	
	inline void rmConfig() {
		std::filesystem::remove("config.json");
	}

	inline void createTemplateExample() {
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
