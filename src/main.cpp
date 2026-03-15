#include <iostream>
#include <vector>
#include <string>
#include <json.hpp>
#include <cpr/cpr.h>
#include <Controller/api.h>
#include <Controller/Core/external.h>
#include <Controller/Core/console.h>
#include <Controller/Core/config.h>
#include <Controller/Web/WebHandler.h>
#include <Controller/ui.h>

#ifdef _WIN32
	void enableVirtualTerminalProcessing() {
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		GetConsoleMode(hOut, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);
	}
#endif

int main(int argc, char* argv[]) {
	bool simulate = false;
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--simulate" || arg == "-s")
			simulate = true;
		else
			console::writeLog("Unknown argument: " + arg, true, 255, 100, 100);
	}

	#ifdef _WIN32
		enableVirtualTerminalProcessing();
	#endif
	console::writeLog("-------------------------------------------------------------  ", true, 255, 255, 255);
	console::writeLog("                    __   ______                __              ", true, 103, 143, 245);
	console::writeLog("  ____  _______  __/ /  /_  __/________ ______/ /_____  _____  ", true, 103, 143, 245);
	console::writeLog(" / __ \\/ ___/ / / / /    / / / ___/ __ `/ ___/ //_/ _ \\/ ___/", true, 122, 103, 245);
	console::writeLog("/ /_/ (__  ) /_/ /_/    / / / /  / /_/ / /__/ ,< /  __/ /      ", true, 246, 12, 250);
	console::writeLog("\\____/____/\\__,_(_)    /_/ /_/   \\__,_/\\___/_/|_|\\___/_/  ", true, 246, 12, 250);
	console::writeLog("                                                               ", true, 246, 12, 250);
	console::writeLog("-------------------------------------------------------------  ", true, 255, 255, 255);
	console::writeLog((std::string)"Version Number: " + (OSU_TRACKER_VERSION), true, 111, 163, 247);
	console::writeLog((std::string)"Signed Update Version: " + (OSU_TRACKER_VERSION_SIGNED), true, 111, 163, 247);
	console::writeLog((std::string)"Release Type: " + (OSU_TRACKER_RELEASE_TYPE), true, 111, 163, 247);
	console::writeLog((std::string)"Build: " + OSU_TRACKER_CMAKE_BUILD_TYPE, true, 111, 163, 247);
	
	if (Web::WebHandler::performUpdateCheck() == 0) {
		return 0;
	}

	bool run = true;
	bool skipInit = false;
	config::data::initDefaults();

	while (run) {
		if (!std::filesystem::exists("config.json")) {
			console::writeLog("Config file not found");
			config::write();
			config::read();
		}
		else {
			console::writeLog("Config file found");
			config::read();
		}
		#ifdef DEBUG_BUILD
			config::readEnv();
		#endif
		if (!std::filesystem::exists("Txt/template")) {
	        if (!std::filesystem::create_directory("Txt/template")) {
				console::writeLog("Failed to create Txt/template", true, 255, 0, 0);
			}
		}
		api::simulateMode = simulate;
		ui::startFetchThread(true);
	    config::createTemplateExample();
		Web::WebHandler::startUiThread();
		#if OSU_TRACKER_WEBSERVER_ENABLE == 1
			run = !Web::WebHandler::start(skipInit); // blocking
			skipInit = true;
			// close ui
		#endif
		console::writeLog("Stopping UI Thread...", true, 255, 0, 0);
		Web::WebHandler::stopUiThread();
		console::writeLog("Stopping Fetch Thread...", true, 255, 0, 0);
		ui::stopFetchThread();
	}
	return 0;
}
