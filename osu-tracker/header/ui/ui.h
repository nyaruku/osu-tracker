#pragma once
#include "../header/ui/ui_imgui.h"

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

class ui {
private:
	static void fetchApiData(bool init) {
		while (fetch) {	
			api::fetch_api_data(init);
			updateFormat();
			config::writeStats();

			std::this_thread::sleep_for(
				std::chrono::milliseconds(config::application::instance().apiInterval)
			);
		}
	}

public:
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
							data.init = formatNumber(data.init);
							if(!noCurrent)
								data.current = formatNumber(data.current);
							if (noDiff)
								break;
							std::string str = formatNumber(data.change, true);
							if (str[0] == '-') {
								data.positive = false;
								api::fetch_api_data(true);
							}
							data.change = str;
							break;
						}
						case config::formatType::f_rank: {
							data.init = formatNumber(data.init, false, "#");
							if (!noCurrent)
								data.current = formatNumber(data.current, false, "#");
							if (noDiff)
								break;
							std::string str = formatNumber(data.change, true);
							if (str[0] == '-') {
								data.positive = false;
							}
							data.change = str;
							break;
						}
						case config::formatType::f_time: {
							data.init = formatPlaytime(data.init);
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
							data.init = formatNumber(data.init);
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
							data.init = formatNumber(data.init, false, "#");
							if (!noCurrent)
								data.current = formatNumber(data.current, false, "#");
							if (noDiff)
								break;
							std::string str = formatNumber(data.change, true);
							if (str[0] == '-') {
								data.positive = false;
							}
							data.change = str;
							break;
						}
						case config::formatType::f_time: {
							data.init = formatPlaytime(data.init);
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
							data.init = formatFloat(data.init);
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
							data.init = formatFloat(data.init) + "%";
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
	static void startFetchThread(bool _init = false) {
		fetch = true;
		fetchThread = std::thread(fetchApiData, _init);
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
