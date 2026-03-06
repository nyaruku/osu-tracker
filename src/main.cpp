#include <iostream>
#include <vector>
#include <string>
#include <json.hpp>
#include <cpr/cpr.h>
#include <Domain/Controller/Core/external.h>
#include <Domain/Controller/api.h>
#include <Domain/Controller/Core/console.h>
#include <Domain/Controller/Core/config.h>
#include <Domain/Controller/Web/WebHandler.h>
#include <Domain/Controller/ui.h>

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
		if (!std::filesystem::exists("tracker_txt/template")) {
	        if (!std::filesystem::create_directory("tracker_txt/template")) {
				console::writeLog("Failed to create tracker_txt/template", true, 255, 0, 0);
			}
		}
		api::fetch_api_data(true);
		ui::startFetchThread();
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
