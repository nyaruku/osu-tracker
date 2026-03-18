#include <iostream>
#include <vector>
#include <string>
#include <json.hpp>
#include <cpr/cpr.h>
#include <Api/Api.h>
#include <Core/Helpers.h>
#include <Core/Console.h>
#include <Core/Config.h>
#include <Web/WebHandler.h>
#include <Core/Session.h>

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
			Core::Console::writeLog("Unknown argument: " + arg, true, 255, 100, 100);
	}

	#ifdef _WIN32
		enableVirtualTerminalProcessing();
	#endif
	Core::Console::writeLog("-------------------------------------------------------------  ", true, 255, 255, 255);
	Core::Console::writeLog("                    __   ______                __              ", true, 103, 143, 245);
	Core::Console::writeLog("  ____  _______  __/ /  /_  __/________ ______/ /_____  _____  ", true, 103, 143, 245);
	Core::Console::writeLog(" / __ \\/ ___/ / / / /    / / / ___/ __ `/ ___/ //_/ _ \\/ ___/", true, 122, 103, 245);
	Core::Console::writeLog("/ /_/ (__  ) /_/ /_/    / / / /  / /_/ / /__/ ,< /  __/ /      ", true, 246, 12, 250);
	Core::Console::writeLog("\\____/____/\\__,_(_)    /_/ /_/   \\__,_/\\___/_/|_|\\___/_/  ", true, 246, 12, 250);
	Core::Console::writeLog("                                                               ", true, 246, 12, 250);
	Core::Console::writeLog("-------------------------------------------------------------  ", true, 255, 255, 255);
	Core::Console::writeLog((std::string)"Version Number: " + (OSU_TRACKER_VERSION), true, 111, 163, 247);
	Core::Console::writeLog((std::string)"Signed Update Version: " + (OSU_TRACKER_VERSION_SIGNED), true, 111, 163, 247);
	Core::Console::writeLog((std::string)"Release Type: " + (OSU_TRACKER_RELEASE_TYPE), true, 111, 163, 247);
	Core::Console::writeLog((std::string)"Build: " + OSU_TRACKER_CMAKE_BUILD_TYPE, true, 111, 163, 247);
	
	if (Web::WebHandler::performUpdateCheck() == 0) {
		return 0;
	}

	bool run = true;
	bool skipInit = false;
	Core::Config::data::initDefaults();

	while (run) {
		if (!std::filesystem::exists("config.json")) {
			Core::Console::writeLog("Config file not found");
			Core::Config::write();
			Core::Config::read();
		}
		else {
			Core::Console::writeLog("Config file found");
			Core::Config::read();
		}
		#ifdef DEBUG_BUILD
			Core::Config::readEnv();
		#endif
		if (!std::filesystem::exists("Txt/template")) {
	        if (!std::filesystem::create_directory("Txt/template")) {
				Core::Console::writeLog("Failed to create Txt/template", true, 255, 0, 0);
			}
		}
		Api::Status::simulateMode = simulate;
		Core::Session::startFetchThread(true);
	    Core::Config::createTemplateExample();
		Web::WebHandler::startUiThread();
		#if OSU_TRACKER_WEBSERVER_ENABLE == 1
			run = !Web::WebHandler::start(skipInit); // blocking
			skipInit = true;
			// close ui
		#endif
		Core::Console::writeLog("Stopping UI Thread...", true, 255, 0, 0);
		Web::WebHandler::stopUiThread();
		Core::Console::writeLog("Stopping Fetch Thread...", true, 255, 0, 0);
		Core::Session::stopFetchThread();
	}
	return 0;
}
