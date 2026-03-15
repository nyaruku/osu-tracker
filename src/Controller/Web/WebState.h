#pragma once
#include <crow.h>
#include <mutex>
#include <unordered_set>
#include <thread>
#include <Controller/Core/console.h>
#include <Controller/ui.h>
#include <Controller/api.h>

namespace Web::WebHandler {

	inline bool shutdown_webServer = false;
	inline static crow::SimpleApp app;
	inline static std::mutex ws_mutex;
	inline static std::unordered_set<crow::websocket::connection*> clients;
	inline long long counter = 0;
	inline std::thread uiThread;

	static void shutdown(bool restart = false) {
		console::writeLog("Web Server termination initiated...", false, 255, 255, 0);
		shutdown_webServer = !restart;
		app.stop();
	}

	static void stopUiThread() {
		console::writeLog("Check if UI Thread is joinable (to close UI)", false, 255, 255, 0);
		if (uiThread.joinable()) {
			console::writeLog("UI Thread is joinable", false, 255, 255, 0);
			ui::close();
			uiThread.join();
		}
		else {
			console::writeLog("UI Thread is not joinable", false, 255, 255, 0);
		}
	}

	static void startUiThread() {
		stopUiThread();
		console::writeLog("Check if UI Thread is joinable (to run UI)", false, 255, 255, 0);
		if (!uiThread.joinable()) {
			console::writeLog("Running UI Thread...", false, 255, 255, 0);
			uiThread = std::thread(ui::open);
		}
		else {
			console::writeLog("UI Thread is running.", false, 255, 255, 0);
		}
	}

	inline int performUpdateCheck() {
		console::writeLog("-------------------------------------------------------------", true, 255, 255, 255);
		console::writeLog("Checking for update...", true, 255, 255, 0);
		if (api::update()) {
			console::writeLog("Updating...", true, 0, 255, 0);
		#if defined(_WIN32)
			system("start update.exe");
		#elif defined(__linux__)
			system("./update &");
		#endif
			return 0;
		}
		else {
			console::writeLog("No updates found.", true, 0, 255, 0);
		}
		console::writeLog("-------------------------------------------------------------", true, 255, 255, 255);
		return 1;
	}
}

