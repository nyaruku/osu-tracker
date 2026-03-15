#pragma once
#include <algorithm>
#include <shared_mutex>
#include <Controller/View/ui_imgui.h>

inline std::atomic<bool> fetch;
inline std::thread fetchThread;

namespace ui {
	inline std::chrono::system_clock::time_point lastFetchCompleted;
	inline std::chrono::milliseconds             lastFetchDuration{ 0 };
	inline std::atomic<int>                      fetchCount{ 0 };
}

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

namespace ui {
	static void updateFormat() {
		config::data::arrFormatted = config::data::arr;

		// Apply user-defined display order
		std::sort(config::data::arrFormatted.begin(), config::data::arrFormatted.end(),
			[](const config::dataEntry& a, const config::dataEntry& b) { return a.sort < b.sort; });

		for (auto& data : config::data::arrFormatted) {

			if (data.init.empty() && !data.single) {
				data.init = "";
				data.change = "";
				continue;
			}
			switch (config::application.server) {
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
			if (data.current.empty()) {
				noCurrent = true;
			}
			switch (data.dataType) {
				case config::dataType::d_int: {
					switch (data.formatType) {
					case config::formatType::f_int: {
						data.init = formatNumber(data.init);
						if(!noCurrent)
							data.current = formatNumber(data.current);
						if (noDiff)
							break;
						std::string str = formatNumber(data.change, true);
						if (!str.empty() && str[0] == '-') {
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
						if (!str.empty() && str[0] == '-') {
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
			case config::dataType::d_longLong: {
				switch (data.formatType) {
					case config::formatType::f_int: {
						data.init = formatNumber(data.init);
						if (!noCurrent)
							data.current = formatNumber(data.current);
						if (noDiff)
							break;
						std::string str = formatNumber(data.change, true);
						if (!str.empty() && str[0] == '-') {
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
						if (!str.empty() && str[0] == '-') {
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
			case config::dataType::d_float: {
				switch (data.formatType) {
					case config::formatType::f_decimal: {
						data.init = formatFloat(data.init);
						if (!noCurrent)
							data.current = formatFloat(data.current);
						if (noDiff)
							break;
						std::string str = formatFloat(data.change, true);
						if (!str.empty() && str[0] == '-') {
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
						std::string str = formatFloat(data.change, true) + "%";
						if (!str.empty() && str[0] == '-') {
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
	static void fetchApiData(bool init) {
		while (fetch) {
			auto t0 = std::chrono::steady_clock::now();
			api::fetch_api_data(init);
			init = false;
			{
				std::unique_lock<std::shared_mutex> lock(config::data::dataMutex);
				updateFormat();
				config::writeStats();
			}
			lastFetchDuration  = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0);
			lastFetchCompleted = std::chrono::system_clock::now();
			fetchCount++;
			std::this_thread::sleep_for(
				std::chrono::milliseconds(config::application.apiInterval)
			);
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

// Stops the current fetch thread, clears data, and restarts with init=true
// Runs asynchronously so the UI thread is not blocked during thread join
inline void ui_restartFetch() {
	static std::atomic<bool> restarting{ false };
	if (restarting.exchange(true)) return;
	std::thread([]() {
		fetch = false;
		if (fetchThread.joinable()) fetchThread.join();
		{
			std::unique_lock<std::shared_mutex> lk(config::data::dataMutex);
			for (auto& e : config::data::arr) { e.init = ""; e.current = ""; e.change = ""; }
			config::data::arrFormatted.clear();
		}
		config::user.avatarUrl = "";
		ui::fetchCount = 0;
		fetch = true;
		fetchThread = std::thread(ui::fetchApiData, true);
		restarting = false;
	}).detach();
}

