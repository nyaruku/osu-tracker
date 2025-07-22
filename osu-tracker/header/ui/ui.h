#pragma once
enum gameMode {
	osu = 0
	, taiko = 1
	, fruits = 2
	, mania = 3
};

enum server {
	bancho = 0
	, titanic = 1
};

struct appC {
	int osuId;
	enum gameMode gameMode;
	enum server server;
};

// Convert C++ config::gameMode to C enum gameMode
enum gameMode to_c_gameMode(config::gameMode gm) {
	switch (gm) {
	case config::gameMode::osu: return osu;
	case config::gameMode::taiko: return taiko;
	case config::gameMode::fruits: return fruits;
	case config::gameMode::mania: return mania;
	default: return osu; // fallback default
	}
}

// Convert C++ config::server to C enum server
enum server to_c_server(config::server sv) {
	switch (sv) {
	case config::server::bancho: return bancho;
	case config::server::titanic: return titanic;
	default: return bancho; // fallback default
	}
}

struct appC to_c_appC(const config::application& app) {
	struct appC c_app;
	c_app.osuId = app.osuId;
	c_app.gameMode = to_c_gameMode(app.gameMode);
	c_app.server = to_c_server(app.server);
	return c_app;
}

struct userC {
	const char* username;
	const char* avatar;
};

struct dataEntryC {
	const char* key;
	const char* name;
	int sort;
	const char* init;
	const char* current;
	const char* change;
	bool positive;
	bool display;
	bool single;
	bool banchoSupport;
	bool titanicSupport;
};

std::atomic<bool> fetch;
static std::thread fetchThread;

std::string formatNumber(const std::string& numStr, bool showPlus=false, std::string c = "") {
	try {
		long long num = std::stoll(numStr);
		bool isNegative = num < 0;
		unsigned long long absNum = isNegative ? -num : num;
		std::string str = std::to_string(absNum);

		// Add commas
		int insertPosition = static_cast<int>(str.length()) - 3;
		while (insertPosition > 0) {
			str.insert(insertPosition, ",");
			insertPosition -= 3;
		}

		if (c == "#")
			str = "#" + str;
		if (isNegative){
			str = "-" + str;
		}
		else {
			if(showPlus)
				str = "+" + str;
		}
		return str;
	}
	catch (const std::exception&) {
		return "Invalid";
	}
}

std::string formatFloat(const std::string& numStr, bool showPlus = false) {
	char* endPtr = nullptr;
	double num = std::strtod(numStr.c_str(), &endPtr);
	if (endPtr == numStr.c_str()) return "Invalid";

	char temp[64];
	snprintf(temp, sizeof(temp), "%.3f", num);
	std::string str(temp);

	bool isNegative = str[0] == '-';
	if (isNegative) {
		str = str.substr(1); // remove the minus for now
	}

	size_t dotPos = str.find('.');
	std::string integerPart = str.substr(0, dotPos);
	std::string decimalPart = str.substr(dotPos); // includes "."

	// Add commas to integer part
	int insertPosition = integerPart.length() - 3;
	while (insertPosition > 0) {
		integerPart.insert(insertPosition, ",");
		insertPosition -= 3;
	}
	if (isNegative) {
		integerPart = "-" + integerPart;
	}
	else {
		if (showPlus)
			integerPart = "+" + integerPart;
	}
	return integerPart + decimalPart;
}

std::string formatPlaytime(const std::string& secondsStr, bool showPlus = false) {
	char* endPtr = nullptr;
	long seconds = std::strtol(secondsStr.c_str(), &endPtr, 10);
	if (endPtr == secondsStr.c_str()) return "Invalid";

	int hours = seconds / 3600;
	int minutes = (seconds % 3600) / 60;
	int secs = seconds % 60;

	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%dh %dm %ds", hours, minutes, secs);
	std::string str = std::string(buffer);
	if (showPlus)
		str = "+" + str;
	return str;
}

#if defined(_WIN32)
	extern "C" int ui_main();
	extern "C" void ui_mainTerminate();
	extern "C" void copyArrayData(struct appC* app, const struct userC* user, const struct dataEntryC* entries, size_t count);
#elif defined(__linux__)
	extern "C" int ui_main();
	extern "C" void ui_mainTerminate();
	extern "C" void copyArrayData(struct appC* app, const struct userC* user, const struct dataEntryC* entries, size_t count);
#endif
class ui {
private:
	static void fetchApiData(bool init) {
		while (fetch) {
			api::fetch_api_data(init);
			copyDataOnly();
			std::this_thread::sleep_for(std::chrono::milliseconds(config::application::instance().apiInterval));
		}
	}
public:
	static void copyDataOnly() {
		updateFormat();
		config::writeStats();

		appC app = to_c_appC(config::application::instance());

		userC user = {
			config::user::instance().username.c_str(),
			config::user::instance().avatar.c_str()
		};

		std::vector<dataEntryC> entriesC;
		entriesC.reserve(config::data::arrFormatted.size());

		for (const config::dataEntry& d : config::data::arrFormatted) {
			dataEntryC c{
				d.key.c_str(),
				d.name.c_str(),
				d.sort,
				d.init.c_str(),
				d.current.c_str(),
				d.change.c_str(),
				d.positive,
				d.display,
				d.single,
				d.banchoSupport,
				d.titanicSupport
			};
			entriesC.push_back(c);
		}

		copyArrayData(&app, &user, entriesC.data(), entriesC.size());

	}
	static void updateFormat() {
		config::data::arrFormatted = config::data::arr;

		for (config::dataEntry& data : config::data::arrFormatted) {

			if (data.init == "" && !data.single) {
				data.init = "";
				data.change = "";
				continue;
			}
			switch (config::application::instance().server) {
				case config::server::bancho:
					if (!data.banchoSupport)
						continue;
					break;
				case config::server::titanic:
					if (!data.titanicSupport)
						continue;
					break;
			}

			bool noDiff = false;
			if ((data.init == data.current) && !data.single) {
				data.change = "";
				noDiff = true;
			}
			bool noCurrent = false;
			if (data.current == "") {
				noCurrent = true;
			}
			switch (data.dataType) {
				case config::dataType::_int: {
					switch (data.formatType) {
						case config::formatType::f_int: {
							if(!noCurrent)
								data.current = formatNumber(data.current);
							if (noDiff)
								break;
							std::string str = formatNumber(data.change, true);
							if (str[0] == '-') {
								data.positive = false;
							}
							data.change = str;
							break;
						}
						case config::formatType::f_rank: {
							if (!noCurrent)
								data.current = formatNumber(data.current, false, "#");
							if (noDiff)
								break;
							std::string str = formatNumber(data.change, true, "#");
							if (str[0] == '-') {
								data.positive = false;
							}
							data.change = str;
							break;
						}
						case config::formatType::f_time: {
							if (!noCurrent)
								data.current = formatPlaytime(data.current);
							if (noDiff)
								break;
							data.change = formatPlaytime(data.change, true);
							break;
						}
					}
					break;
				}
				case config::dataType::_longLong: {
					switch (data.formatType) {
						case config::formatType::f_int: {
							if (!noCurrent)
								data.current = formatNumber(data.current);
							if (noDiff)
								break;
							std::string str = formatNumber(data.change, true);
							if (str[0] == '-') {
								data.positive = false;
							}
							data.change = str;
							break;
						}
						case config::formatType::f_rank: {
							if (!noCurrent)
								data.current = formatNumber(data.current, false, "#");
							if (noDiff)
								break;
							std::string str = formatNumber(data.change, true, "#");
							if (str[0] == '-') {
								data.positive = false;
							}
							data.change = str;
							break;
						}
						case config::formatType::f_time: {
							if (!noCurrent)
								data.current = formatPlaytime(data.current);
							if (noDiff)
								break;
							data.change = formatPlaytime(data.change, true);
							break;
						}
					}
					break;
				}
				case config::dataType::_float: {
					switch (data.formatType) {
						case config::formatType::f_decimal: {
							if (!noCurrent)
								data.current = formatFloat(data.current);
							if (noDiff)
								break;
							std::string str = formatFloat(data.change, true);
							if (str[0] == '-') {
								data.positive = false;
							}
							data.change = str;
							break;
						}
						case config::formatType::f_percent: {
							if (!noCurrent)
								data.current = formatFloat(data.current) + "%";
							if (noDiff)
								break;
							std::string str = formatFloat(data.change, true) + "%";;
							if (str[0] == '-') {
								data.positive = false;
							}
							data.change = str;
							break;
						}
					}
					break;
				}
			}
		}
	}
	static inline std::atomic<bool> isOpen;
	static void startFetchThread() {
		fetch = true;
		fetchThread = std::thread(fetchApiData, false);
	}
	static int stopFetchThread() {
		fetch = false;
		if (fetchThread.joinable()) {
			fetchThread.join();
		}
		return 1;
	}
	static void close() {
		ui_mainTerminate();
		isOpen = false;
	}
	static int open() {
		isOpen = true;
		int result = ui_main();
		isOpen = false;
		return result;
	}
};
