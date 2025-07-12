#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
#pragma GCC diagnostic pop


#pragma once
#include <utility>
#include <string>
#include "console.h"
#include <algorithm>

constexpr uint32_t fnv1a(const char* str) {
	uint32_t hash = 2166136261u;
	while (*str) {
		hash ^= static_cast<uint32_t>(*str++);
		hash *= 16777619u;
	}
	return hash;
}

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

class config {
public:
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

		static application& instance() {
			static application ctx;
			return ctx;
		}

		#define DECLARE(type, name, default_val) type name = default_val;
			applicationFields(DECLARE)
		#undef DECLARE

		// Convert to array
		std::vector<std::tuple<std::string, std::string>> toArray() const {
			std::vector<std::tuple<std::string, std::string>> result;
			#define TO_STRING(type, name, default_val) result.emplace_back(#name, to_string(name));
				applicationFields(TO_STRING)
			#undef TO_STRING
				return result;
		}
		std::string get(const std::string& key) const {
			#define GET_CASE(type, name, default_val) if (key == #name) return to_string(name);
				applicationFields(GET_CASE)
			#undef GET_CASE
			throw std::invalid_argument("Invalid key: " + key);
		}
		static void set(const std::string& key, const std::string& value) {
			application& ctx = instance();
			#define SET_CASE(type, name, default_val) if (key == #name) { from_string(value, ctx.name); return; }
				applicationFields(SET_CASE)
			#undef SET_CASE
			throw std::invalid_argument("Invalid key: " + key);
		}
	private:
		// Generic string conversion
		template<typename T>
		static std::string to_string(const T& val) {
			if constexpr (std::is_enum_v<T>) {
				return std::to_string(static_cast<int>(val));
			}
			else {
				return std::to_string(val);
			}
		}

		static std::string to_string(const std::string& val) {
			return val;
		}

		// Parsing from string
		template<typename T>
		static void from_string(const std::string& str, T& out) {
			std::istringstream ss(str);
			ss >> out;
		}

		static void from_string(const std::string& str, std::string& out) {
			out = str;
		}

		static void from_string(const std::string& str, config::gameMode& out) {
			out = static_cast<config::gameMode>(std::stoi(str));
		}

		static void from_string(const std::string& str, config::server& out) {
			out = static_cast<config::server>(std::stoi(str));
		}
	};

	struct user {

		static user& instance() {
			static user ctx;
			return ctx;
		}

		std::string username = "";
		std::string avatar = "";

		std::vector<std::tuple<std::string, std::string>> toArray() const {
			return {
				{"username", username}
				,{ "avatar", avatar}
			};
		}
	};

	enum class dataType {
		_string = 0
		,_int = 1
		,_float = 2
		,_longLong = 3
	};

	enum class formatType {
		f_string = 0
		,f_int = 1
		,f_decimal = 2
		,f_rank = 3
		,f_time = 4
		,f_percent = 5
	};
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchanges-meaning"
#endif

	struct dataEntry {
		std::string key;
		std::string name;
		int sort;
		std::string init;
		std::string current;
		std::string change;
		dataType dataType;
		formatType formatType;
		bool positive;
		bool display;
		bool single;
		bool banchoSupport;
		bool titanicSupport;

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
				,{"banchoSupport", ext::bool2str(banchoSupport)}
				,{"titanicSupport", ext::bool2str(titanicSupport)}
			};
		}
	};
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
	
	class data {
	public:
		static inline std::vector<dataEntry> arr {																				   //bancho  titanic
			{"level",		"Level",			1, "", "",	"", dataType::_float,	formatType::f_decimal,	true,	true,	false,	true,	true}
			,{"rankedScore","Ranked Score",		2, "", "",	"", dataType::_longLong,formatType::f_int,		true,	true,	false,	true,	true}
			,{"totalScore",	"Total Score",		3, "", "",	"", dataType::_longLong,formatType::f_int,		true,	true,	false,	true,	true}
			,{"ppRank",		"PP Rank",			4, "", "",	"", dataType::_int,		formatType::f_rank,		true,	true,	false,	true,	true}
			,{"pp",			"PP",				5, "", "",	"", dataType::_float,	formatType::f_decimal,	true,	true,	false,	true,	true}
			,{"ppv1",		"PPv1",				6, "", "",	"", dataType::_float,	formatType::f_decimal,	true,	true,	false,	false,	true}
			,{"acc",		"Accuracy",			7, "", "",	"", dataType::_float,	formatType::f_percent,	true,	true,	false,	true,	true}
			,{"playtime",	"Play Time",		8, "", "",	"", dataType::_longLong,formatType::f_time,		true,	true,	false,	true,	true} //long long just in case
			,{"playcount",	"Play Count",		9, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"totalHits",	"Total Hits",		10, "", "",	"", dataType::_longLong,formatType::f_int,		true,	true,	false,	false,	true} //long long just in	case

			,{"silverSS",	"Rank SSH",			11, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"goldSS",		"Rank SS",			12, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"silverS",	"Rank SH",			13, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"goldS",		"Rank S",			14, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"a",			"Rank A",			15, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"b",			"Rank B",			16, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"c",			"Rank C",			17, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"d",			"Rank D",			18, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"totalSS",	"Total SS",			19, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"totalS",		"Total S",			20, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}

			,{"clears",		"Profile Clears",	21, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"totalClears","Total Clears",		22, "", "",	"", dataType::_int,		formatType::f_int,		true,	true,	false,	true,	true}
			,{"completion",	"Completion%",		23, "", "",	"", dataType::_float,	formatType::f_percent,	true,	true,	false,	true,	true}
			,{"scoreRank",	"Score Rank",		24, "", "",	"", dataType::_int,		formatType::f_rank,		true,	true,	false,	true,	false}

			,{"targetRank",	"Target Rank",		25, "", "",	"", dataType::_int,		formatType::f_rank,		false,	true,	true,	true,	true}
			,{"targetUser",	"Target Player",	26, "", "",	"", dataType::_string,	formatType::f_string,	false,	true,	true,	true,	true}
			,{"targetScore","Target Score",		27, "", "",	"", dataType::_longLong,formatType::f_int,		false,	true,	true,	true,	true}
		};
		static inline std::vector<dataEntry> arrFormatted;

		static constexpr int getIndex(const char* key) {
		#define INDEX_CASE(name) GEN_CASE(name, __COUNTER__)
			FOREACH_DATA_KEY(INDEX_CASE)
				return -1;
		}
	};

	enum fileHeader {
		None
		,Main
		,Display
		,Sort
	};
	static fileHeader resolve(std::string str) {
		if (str == "[Main]") return Main;
		if (str == "[Display]") return Display;
		if (str == "[Sort]") return Sort;
		return None;
	}

	// this is basically a csv generator with headers and comments
	static void writeConfig() {
		console::writeLog("Writing config file...");
		std::string input;
		input += "//PLEASE DONT MANUALLY EDIT THIS FILE\n"
			"//(you still can,but you might break it)\n"
			"//(If you want to reset all settings, just delete this file)\n\n";
		input += "[Main]";
		for (const auto& [key, value] : config::application::instance().toArray()) {
			input += "\n" + key + ";" + value;
		}
		input += "\n\n[Display]";
		for (const config::dataEntry _vecData : config::data::arr) {
			input += "\n" + _vecData.key + ";" + ext::bool2str(_vecData.display);
		}
		input += "\n\n[Sort]\n" + std::to_string(config::data::arr[0].sort);
		for (size_t _idx = 1; _idx < config::data::arr.size(); _idx++) {
			input += ";" + std::to_string(config::data::arr[_idx].sort);
		}
		std::ofstream file;
		file.open("config.txt");
		file << input;
		file.close();
		console::writeLog("Config written");
	}

	static void readConfig() {
		if (std::ifstream file{ "config.txt" }; file.is_open()) {
			console::writeLog("Reading config file...");
			int configHeader = -1;
			for (std::string line; std::getline(file, line); ){
				if (line.starts_with("//") || line.empty())
					continue;
				if (line.starts_with("[")) {
					switch (resolve(line)) {
						case Main:
							configHeader = 0;
							break;
						case Display:
							configHeader = 1;
							break;
						case Sort:
							configHeader = 2;
							break;
					}
					continue;
				}
				std::string key, value;
				switch (configHeader) {
					case 0:
						std::tie(key, value) = ext::split2tuple(line, ';');
						config::application::set(key, value);
						break;
					case 1:
						std::tie(key, value) = ext::split2tuple(line, ';');
						config::data::arr[config::data::getIndex(key.c_str())].display = ext::str2bool(value);
						break;
					case 2:
						std::vector<std::string> _arr = ext::split2vector(line, ';');
						for (size_t _idx = 0; _idx < _arr.size(); _idx++) {
							config::data::arr[_idx].sort = std::stoi(_arr[_idx]);
						}
						break;
				}
			}
			file.close();
		}
		else {
			console::writeLog("Error reading config file", true, 255, 0, 0);
		}
	}

	static void rmConfig() {
		std::filesystem::remove("config.txt");
	}
};