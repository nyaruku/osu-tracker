#pragma once
#include <crow.h>
#include <mutex>
#include <unordered_set>
#include <thread>
#include <Core/Console.h>
#include <Core/Session.h>
#include <Api/Api.h>

namespace Web::WebHandler {

	inline bool shutdown_webServer = false;
	inline static crow::SimpleApp app;
	inline static std::mutex ws_mutex;
	inline static std::unordered_set<crow::websocket::connection*> clients;
	inline long long counter = 0;
	inline std::thread uiThread;

	static void shutdown(bool restart = false) {
		Core::Console::writeLog("Web Server termination initiated...", false, 255, 255, 0);
		shutdown_webServer = !restart;
		app.stop();
	}

	static void stopUiThread() {
		Core::Console::writeLog("Check if UI Thread is joinable (to close UI)", false, 255, 255, 0);
		if (uiThread.joinable()) {
			Core::Console::writeLog("UI Thread is joinable", false, 255, 255, 0);
			Core::Session::CloseWindow();
			uiThread.join();
		}
		else {
			Core::Console::writeLog("UI Thread is not joinable", false, 255, 255, 0);
		}
	}

	static void startUiThread() {
		stopUiThread();
		Core::Console::writeLog("Check if UI Thread is joinable (to run UI)", false, 255, 255, 0);
		if (!uiThread.joinable()) {
			Core::Console::writeLog("Running UI Thread...", false, 255, 255, 0);
			uiThread = std::thread(Core::Session::OpenWindow);
		}
		else {
			Core::Console::writeLog("UI Thread is running.", false, 255, 255, 0);
		}
	}

	inline int performUpdateCheck() {
		Core::Console::writeLog("-------------------------------------------------------------", true, 255, 255, 255);
		Core::Console::writeLog("Checking for update...", true, 255, 255, 0);
		if (Api::Update::update()) {
			Core::Console::writeLog("Updating...", true, 0, 255, 0);
		#if defined(_WIN32)
			system("start update.exe");
		#elif defined(__linux__)
			system("./update &");
		#endif
			return 0;
		}
		else {
			Core::Console::writeLog("No updates found.", true, 0, 255, 0);
		}
		Core::Console::writeLog("-------------------------------------------------------------", true, 255, 255, 255);
		return 1;
	}
}