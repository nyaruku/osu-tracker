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

#ifdef _WIN32
	void enableVirtualTerminalProcessing() {
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		GetConsoleMode(hOut, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);
	}
#endif

void printHeader() {
	writeLog("-------------------------------------------------------------", true,255,255,255);
	writeLog("                    __   ______                __            ",		true, 103, 143, 245);
	writeLog("  ____  _______  __/ /  /_  __/________ ______/ /_____  _____",		true, 103, 143, 245);
	writeLog(" / __ \\/ ___/ / / / /    / / / ___/ __ `/ ___/ //_/ _ \\/ ___/",		true, 122, 103, 245);
	writeLog("/ /_/ (__  ) /_/ /_/    / / / /  / /_/ / /__/ ,< /  __/ /    ",		true, 246, 12, 250);
	writeLog("\\____/____/\\__,_(_)    /_/ /_/   \\__,_/\\___/_/|_|\\___/_/     ",	true, 246, 12, 250);
	writeLog("                                                             ",		true, 246, 12, 250);
	writeLog("-------------------------------------------------------------", true, 255, 255, 255);
	writeLog((std::string)"Version Number: " + (OSU_TRACKER_VERSION), true, 111, 163, 247);
	writeLog((std::string)"Release Type: " + (OSU_TRACKER_RELEASE_TYPE), true, 111, 163, 247);
#ifdef DEBUG_BUILD
	writeLog((std::string)"Build: DEBUG", true, 111, 163, 247);
#else
	writeLog((std::string)"Build: RELEASE", true, 111, 163, 247);
#endif
	writeLog("-------------------------------------------------------------", true, 255, 255, 255);
}

int main()
{
	#ifdef _WIN32 
		enableVirtualTerminalProcessing(); //poopoo cmd.exe 
	#endif
	printHeader();

	bool run = true;
	bool skipInit = false;
	while (run) {
		if (!std::filesystem::exists("config.txt")) {
			writeLog("Config file not found");
			config::writeConfig();
			config::readConfig();
		}
		else {
			writeLog("Config file found");
			config::readConfig();
			config::writeConfig();
		}
		#if OSU_TRACKER_ENABLE_WEBSERVER == 1
			fetch_api_data(true);
			run = !webserver_start(skipInit); // blocking
			skipInit = true;
		#endif
		}
	return 0;
}