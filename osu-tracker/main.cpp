// suppress warnings bc they spam output
#pragma warning( disable : 4305)
#pragma warning( disable : 4566)

#include <iostream>
#include <vector>
#include <string>
#include <json.hpp>
#include <cpr/cpr.h>
#include "../header/ext.h"
#include "../header/api.h"
#include "../header/console.h"
#include "../header/config.h"
#include "../header/webserver.h"
#include "../header/ui/ui.h"

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
		enableVirtualTerminalProcessing(); //poopoo cmd.exe 
	#endif
	console::writeLog("-------------------------------------------------------------", true, 255, 255, 255);
	console::writeLog("                    __   ______                __            ", true, 103, 143, 245);
	console::writeLog("  ____  _______  __/ /  /_  __/________ ______/ /_____  _____", true, 103, 143, 245);
	console::writeLog(" / __ \\/ ___/ / / / /    / / / ___/ __ `/ ___/ //_/ _ \\/ ___/", true, 122, 103, 245);
	console::writeLog("/ /_/ (__  ) /_/ /_/    / / / /  / /_/ / /__/ ,< /  __/ /    ", true, 246, 12, 250);
	console::writeLog("\\____/____/\\__,_(_)    /_/ /_/   \\__,_/\\___/_/|_|\\___/_/     ", true, 246, 12, 250);
	console::writeLog("                                                             ", true, 246, 12, 250);
	console::writeLog("-------------------------------------------------------------", true, 255, 255, 255);
	console::writeLog((std::string)"Version Number: " + (OSU_TRACKER_VERSION), true, 111, 163, 247);
	console::writeLog((std::string)"Signed Update Version: " + (OSU_TRACKER_VERSION_SIGNED), true, 111, 163, 247);
	console::writeLog((std::string)"Release Type: " + (OSU_TRACKER_RELEASE_TYPE), true, 111, 163, 247);
	console::writeLog((std::string)"Build: " + OSU_TRACKER_CMAKE_BUILD_TYPE, true, 111, 163, 247);
	
	if (webserver::instance().performUpdateCheck() == 0)
		return 0;
	bool run = true;
	bool skipInit = false;	
	while (run) {
		if (!std::filesystem::exists("config.json")) {
			console::writeLog("Config file not found");
			config::writeConfig();
			config::readConfig();
		}
		else {
			console::writeLog("Config file found");
			config::readConfig();
			config::writeConfig();
		}
		if (!std::filesystem::exists("tracker_txt/template")) {
	        if (std::filesystem::create_directory("tracker_txt/template")) {
				// no output
			} else {
				console::writeLog("Failed to create tracker_txt/template", true, 255, 0, 0);
			}
		}
		api::fetch_api_data(true);
		ui::startFetchThread();
		ui::copyDataOnly();
	    config::createTemplateExample();
		webserver::startUiThread();
		#if OSU_TRACKER_ENABLE_WEBSERVER == 1
			run = !webserver::start(skipInit); // blocking
			skipInit = true;
			// close ui
		#endif
		console::writeLog("Stopping UI Thread...", true, 255, 0, 0);
		webserver::stopUiThread();
		console::writeLog("Stopping Fetch Thread...", true, 255, 0, 0);
		ui::stopFetchThread();
	}
	return 0;
}
