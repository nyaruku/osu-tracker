#define CROW_STATIC_DIRECTORY "www/static/"
#define CROW_ENFORCE_WS_SPEC

#include "crow.h"
#include "json.hpp"
#include <filesystem>
#include <unordered_set>
#include <mutex>
#include "config.h"
#include "ui/ui.h"

class CustomLogger : public crow::ILogHandler {
public:
	CustomLogger() {}
	void log(std::string message, crow::LogLevel level) {
		auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm* timeInfo = localtime(&currentTime);;
		char dateBuffer[20];
		strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", timeInfo);
		char timeBuffer[9];
		strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", timeInfo);
		switch (level) {
			#ifdef DEBUG_BUILD
				case crow::LogLevel::Debug:
					console::setColorRGB_f(100, 100, 100);
					std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
					console::resetColor();

					std::cout << "[";
					console::setColor(console::conCol::b_defaultColor, console::conCol::f_white);
					std::cout << "Debug";
					console::resetColor();
					std::cout << "] ";
					console::setColorRGB_f(255, 255, 255);
					std::cout << message << "\n";
					console::resetColor();
					break;
				case crow::LogLevel::Info:
					console::setColorRGB_f(100, 100, 100);
					std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
					console::resetColor();

					std::cout << "[";
					console::setColor(console::conCol::b_defaultColor, console::conCol::f_cyan);
					std::cout << "Info";
					console::resetColor();
					std::cout << "] ";
					console::setColorRGB_f(255, 255, 255);
					std::cout << message << "\n";
					console::resetColor();
					break;
			#endif
			case crow::LogLevel::Warning: 
				console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				console::resetColor();

				std::cout << "[";
				console::setColor(console::conCol::b_defaultColor, console::conCol::f_yellow);
				std::cout << "Warning";
				console::resetColor();
				std::cout << "] ";
				console::setColorRGB_f(255, 255, 255);
				std::cout << message << "\n";
				console::resetColor();
				break;
			case crow::LogLevel::Error: 
				console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				console::resetColor();

				std::cout << "[";
				console::setColor(console::conCol::b_defaultColor, console::conCol::f_red);
				std::cout << "Error";
				console::resetColor();
				std::cout << "] ";
				console::setColorRGB_f(255, 255, 255);
				std::cout << message << "\n";
				console::resetColor();
				break;
			case crow::LogLevel::Critical: 
				console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				console::resetColor();

				console::setColor(console::conCol::b_red, console::conCol::f_white);
				std::cout << "[Critical]";
				std::cout << "]";
				console::setColorRGB_f(255, 255, 255);
				std::cout << " " << message << "\n";
				console::resetColor();
				break;
		}
		std::string logLevel;
		// another switch to keep Debug/Info Log in release build for log page
		switch(level){
			case crow::LogLevel::Debug:		logLevel = "Debug";		break;
			case crow::LogLevel::Info:		logLevel = "Info";		break;
			case crow::LogLevel::Warning:	logLevel = "Warning";	break;
			case crow::LogLevel::Error:		logLevel = "Error";		break;
			case crow::LogLevel::Critical:	logLevel = "Critical";	break;
		}
		std::stringstream ss;
		ss << (std::string)dateBuffer + " " + timeBuffer << " [" << logLevel <<"] " << message;
		console::vec_log.push_back(ss.str());
	}
};

namespace webserver {
	bool shutdown_webServer = false;
	using json = nlohmann::json;
	inline static crow::SimpleApp app;
	inline static std::mutex ws_mutex;
	inline static std::unordered_set<crow::websocket::connection*> clients;
	inline static std::unordered_set<crow::websocket::connection*> clients_settings;
	long long counter = 0;

	// send Toast to client
	json sendToast(std::string msg) {
		json _j;
		_j["cmd"] = "toast";
		_j["msg"] = msg;
		return _j;
	}

	std::thread uiThread;

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


	int performUpdateCheck() {
		console::writeLog("-------------------------------------------------------------", true, 255, 255, 255);
		console::writeLog("Checking for update...", true, 255, 255, 0);
		if (api::update()) {
			console::writeLog("Updaing...", true, 0, 255, 0);
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

	static void shutdown(bool restart = false) {
		console::writeLog("Web Server termination initiated...", false, 255, 255, 0);
		shutdown_webServer = !restart;
		app.stop();
	}

	/*
		- blocking function
		return true - shutdown
		return false - restart
	*/
	static bool start(bool skipInit = false)
	{
		config::user user;
		config::application application;

		// Skip Initialization -> crash
		CustomLogger logger;
		crow::logger::setHandler(&logger);

		if (!skipInit)
		{
			// Get the path to the "www" directory relative to the executable
			std::string www_path = std::filesystem::current_path().string() + "/www/";
			crow::mustache::set_base("./www/");
			crow::mustache::set_global_base("./www/");

			// websocket routes
			CROW_WEBSOCKET_ROUTE(app, "/ws/main/")
			.onopen([&](crow::websocket::connection& conn) {
				// Add & Open WS Connection
				std::lock_guard<std::mutex> _(ws_mutex);
				clients.insert(&conn);
			})
			.onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
				// Close Websocket Connection
				std::lock_guard<std::mutex> _(ws_mutex);
				clients.erase(&conn);
			})
			.onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
				std::lock_guard<std::mutex> _(ws_mutex);
				nlohmann::json j = json::parse(data);

				std::string cmd = j["cmd"];
				console::writeLog("Command received: " + cmd);
		
				if (cmd[0] == '#') {
					if (cmd == "#restart")
						shutdown(true);
					if (cmd == "#shutdown")
						shutdown(false);
					if (cmd == "#open_ui")
						startUiThread();
					if (cmd == "#count") {
						counter++;
						conn.send_text(sendToast(std::to_string(counter)).dump());
					}
					if (cmd == "#update") {
						if (performUpdateCheck() == 0)
							shutdown(false);
						conn.send_text(sendToast((std::string)"No updates found.").dump());
					}
				}
			});

			CROW_WEBSOCKET_ROUTE(app, "/ws/settings/")
			.onopen([&](crow::websocket::connection& conn) {
				// Add & Open WS Connection
				std::lock_guard<std::mutex> _(ws_mutex);
				clients_settings.insert(&conn);
			})
			.onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
				// Close Websocket Connection
				std::lock_guard<std::mutex> _(ws_mutex);
				clients_settings.erase(&conn);
			})
			.onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
				std::lock_guard<std::mutex> _(ws_mutex);
				nlohmann::json j = json::parse(data);
				std::string cmd = j["cmd"];
				console::writeLog("Command received: " + cmd);

				if (cmd[0] == '#') {
					if (cmd == "#saveSettings") {
						bool resetSession = false;
						if 
							(
							j["msg"]["applicationConfig"][0]["value"] != application.get("osuId")
							|| j["msg"]["applicationConfig"][1]["value"] != config::application::instance().get("clientId")
							|| j["msg"]["applicationConfig"][2]["value"] != config::application::instance().get("clientSecret")
							|| j["msg"]["applicationConfig"][4]["value"] != config::application::instance().get("gameMode")
							|| j["msg"]["applicationConfig"][5]["value"] != config::application::instance().get("server")
							) 
						{
							// reset tracker data
							resetSession = true;
						}
						for (const auto& item : j["msg"]["applicationConfig"]) {
							config::application::set(item["key"], item["value"]);
						}
						// server independent
						for (const auto& item : j["msg"]["trackerConfig"]) {
							config::data::arr[config::data::getIndex(item["key"].get<std::string>().c_str())].display = item["value"];
						}
						if (resetSession) {
							console::writeLog("Stopping API Fetch Thread (may take some seconds, depending on interval)", true, 255, 255, 0);
							ui::stopFetchThread();
							ui::startFetchThread(true);	
						}
						config::writeConfig();
					}
					if (cmd == "#resetSettings") {
						config::rmConfig();
						shutdown(true); //restart web server
					}
					if (cmd == "#resetSession") {
						//api::fetch_api_data(true);
						console::writeLog("Stopping API Fetch Thread (may take some seconds, depending on interval)", true, 255, 255, 0);
						ui::stopFetchThread();
						ui::startFetchThread(true);
					}
				}
			});

			CROW_WEBSOCKET_ROUTE(app, "/ws/log/")
			.onopen([&](crow::websocket::connection& conn) {
				// Add & Open WS Connection
				std::lock_guard<std::mutex> _(ws_mutex);
				clients_settings.insert(&conn);
				nlohmann::json _j;
				for (size_t t = 0; t < console::instance().vec_log.size(); t++) {
					_j[t] = console::instance().vec_log[t];
				}
				conn.send_text(_j.dump());
			})
			.onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
				// Close Websocket Connection
				std::lock_guard<std::mutex> _(ws_mutex);
				clients_settings.erase(&conn);
			});

			CROW_WEBSOCKET_ROUTE(app, "/ws/overlay-data/")
			.onopen([&](crow::websocket::connection& conn) {
				// Add & Open WS Connection
				std::lock_guard<std::mutex> _(ws_mutex);
				clients.insert(&conn);

				// Send current overlay stats immediately
				nlohmann::json j;
				for (const auto& row : config::data::arr) {
					// raw
					j[row.key + "_init_raw"] = row.init;
					j[row.key + "_change_raw"] = row.change;
					j[row.key + "_current_raw"] = row.current;

					// formatted
					int idx = config::data::getIndex(row.key.c_str());
					if (idx >= 0 && idx < static_cast<int>(config::data::arrFormatted.size()) &&
						config::data::arrFormatted[idx].key == row.key)
					{
						const auto& f = config::data::arrFormatted[idx];
						j[row.key + "_init"] = f.init;
						j[row.key + "_change"] = f.change;
						j[row.key + "_current"] = f.current;
					}
					else
					{	// happens on launch if fetch thread has not started fast enough and has not filled arrFormatted fast enough
						// somehow was unnoticed on linux
						console::writeLog("Vector missmatch [IGNORE THIS ERROR]", false, 255, 0, 0);
					}
				}

				conn.send_text(j.dump());
			})
			.onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
				// Remove client
				std::lock_guard<std::mutex> _(ws_mutex);
				clients.erase(&conn);
			})
			.onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
				std::lock_guard<std::mutex> _(ws_mutex);

				try {
					nlohmann::json stats;
					for (const auto& row : config::data::arr) {
						stats[row.key + "_init_raw"] = row.init;
						stats[row.key + "_change_raw"] = row.change;
						stats[row.key + "_current_raw"] = row.current;

						const auto& f = config::data::arrFormatted[config::data::getIndex(row.key.c_str())];
						stats[row.key + "_init"] = f.init;
						stats[row.key + "_change"] = f.change;
						stats[row.key + "_current"] = f.current;
					}
					conn.send_text(stats.dump());
				} catch (const std::exception& e) {
					console::writeLog("Overlay WS parse error: " + std::string(e.what()), true, 255, 0, 0);
				}
			});

			// Page routing
			CROW_ROUTE(app, "/")([]() {
				crow::mustache::context ctx;
				ctx["title"] = OSU_TRACKER_NAME;
				ctx["version"] = OSU_TRACKER_VERSION;
				ctx["hostname"] = OSU_TRACKER_WEBSERVER_IP;
				ctx["port"] = OSU_TRACKER_WEBSERVER_PORT;

				auto page = crow::mustache::load("index.html").render(ctx);
				return page;
			});

			CROW_ROUTE(app, "/settings")([]() {
				crow::mustache::context ctx;

				// global context
				ctx["title"] = OSU_TRACKER_NAME;
				ctx["version"] = OSU_TRACKER_VERSION;
				ctx["hostname"] = OSU_TRACKER_WEBSERVER_IP;
				ctx["port"] = OSU_TRACKER_WEBSERVER_PORT;

				// config
				ctx["osu_id_name"] = "osu! User ID";
				ctx["osu_id_val"] = config::application::instance().get("osuId");
				ctx["osu_id_desc"] = "Your osu! user id.";

				ctx["client_id_name"] = "Client ID";
				ctx["client_id_val"] = config::application::instance().get("clientId");
				ctx["client_id_desc"] = "osu! API V2 Client ID.";

				ctx["client_secret_name"] = "Client Secret";
				ctx["client_secret_val"] = config::application::instance().get("clientSecret");
				ctx["client_secret_desc"] = "osu! API V2 Client Secret ( DO NOT SHARE )!";

				ctx["api_refreshInterval_name"] = "API Refresh Interval";
				ctx["api_refreshInterval_val"] = config::application::instance().get("apiInterval");
				ctx["api_refreshInterval_desc"] = "Time in (ms) till api fetches again in the loop.";

				ctx["gameMode_name"] = "Game Mode";
				ctx["gameMode_val_" + std::to_string(static_cast<int>(config::application::instance().gameMode))] = "selected";
				ctx["gameMode_desc"] = "Game Mode to track.";

				ctx["server_name"] = "Server";
				ctx["server_val_" + std::to_string(static_cast<int>(config::application::instance().server))] = "selected";
				ctx["server_desc"] = "Which Server you want to track, bancho or a private server.";


				// bootstrap: "display: none" class
				// hide settings not related to private server
				switch (config::application::instance().server) {
					case config::server::titanic:
						ctx["hide_on_privateServer"] = "d-none";
						break;
				}

				std::vector<crow::json::wvalue> elements;
				switch (config::application::instance().server) {
				case config::server::bancho:
					ctx["tracker_config_name"] = "Bancho Tracker Config";
					for (const config::dataEntry _vecData : config::data::arr) {
						if (!_vecData.banchoSupport)
							continue;
						crow::json::wvalue el;
						el["id"] = _vecData.key;
						el["label"] = _vecData.name;
						if(_vecData.display)
							el["checked"] = " checked";
						elements.push_back(std::move(el));
					}				
					break;
				case config::server::titanic:
					ctx["tracker_config_name"] = "Titanic Tracker Config";
					for (const config::dataEntry _vecData : config::data::arr) {
						if (!_vecData.titanicSupport)
							continue;
						crow::json::wvalue el;
						el["id"] = _vecData.key;
						el["label"] = _vecData.name;
						if (_vecData.display)
							el["checked"] = " checked";
						elements.push_back(std::move(el));
					}
					break;
				}
				ctx["trackerConfig"] = std::move(elements);

				auto page = crow::mustache::load("settings.html").render(ctx);
				return page;
			});
			
			CROW_ROUTE(app, "/log")([]() {
				crow::mustache::context ctx;
				ctx["title"] = OSU_TRACKER_NAME;
				ctx["version"] = OSU_TRACKER_VERSION;
				ctx["hostname"] = OSU_TRACKER_WEBSERVER_IP;
				ctx["port"] = OSU_TRACKER_WEBSERVER_PORT;
				auto page = crow::mustache::load("log.html").render(ctx);
				return page;

			});

			CROW_ROUTE(app, "/info")([](crow::SimpleApp app) {
				crow::mustache::context ctx;
				ctx["title"] = OSU_TRACKER_NAME;
				ctx["version"] = OSU_TRACKER_VERSION;
				ctx["hostname"] = OSU_TRACKER_WEBSERVER_IP;
				ctx["port"] = OSU_TRACKER_WEBSERVER_PORT;

				// PROJECT
				ctx["thread_count"] = std::to_string(app.concurrency());
				ctx["github"] = "https://github.com/nyaruku/osu-tracker";
				ctx["creator"] = "Railgun";
				ctx["profile"] = "https://osu.ppy.sh/users/13817114";
				ctx["discord"] = "https://discord.gg/amj5vBjTQU";
		
				// BUILD INFO
				ctx["build_type"] = OSU_TRACKER_CMAKE_BUILD_TYPE;
				ctx["release_type"] = OSU_TRACKER_RELEASE_TYPE;

				// Libraries
				ctx["crow_version"] = crow::VERSION;
				ctx["crow_repo_url"] = "https://github.com/CrowCpp/Crow/tree/e8b125159ec5e61384187e36bbf6d14f1fad504f";

				ctx["libcurl_version"] = LIBCURL_VERSION;
				ctx["libcurl_repo_url"] = "https://github.com/curl/curl";

				ctx["asio_version"] = "10.30.02";
				ctx["asio_repo_url"] = "https://github.com/chriskohlhoff/asio";

				ctx["cpr_version"] = CPR_VERSION;
				ctx["cpr_repo_url"] = "https://github.com/libcpr/cpr";


				const char* ZLIB_VERSION;

				// Get ZLIB version info
				// Somehow only returns version on Linux, idk why, maybe fix ?
				curl_version_info_data* version_info = curl_version_info(CURLVERSION_NOW);
				if (version_info->features & CURL_VERSION_LIBZ) {
					ZLIB_VERSION = version_info->libz_version;
				}
				else {
					ZLIB_VERSION = "Unknown";
				}


				ctx["zlib_version"] = ZLIB_VERSION;
				ctx["zlib_repo_url"] = "https://github.com/madler/zlib";
		
				ctx["nlohmannJson_version"] = "3.11.2";
				ctx["nlohmannJson_repo_url"] = "https://github.com/nlohmann/json";

				// CMAKE INFO
				ctx["OSU_TRACKER_CMAKE_GENERATOR"] = OSU_TRACKER_CMAKE_GENERATOR;
				ctx["OSU_TRACKER_CMAKE_CXX_STANDARD"] = OSU_TRACKER_CMAKE_CXX_STANDARD;
				ctx["OSU_TRACKER_CMAKE_VERSION"] = OSU_TRACKER_CMAKE_VERSION;
				ctx["OSU_TRACKER_CMAKE_MINIMUM_REQUIRED_VERSION"] = OSU_TRACKER_CMAKE_MINIMUM_REQUIRED_VERSION;
				ctx["OSU_TRACKER_CMAKE_CXX_COMPILER_ID"] = OSU_TRACKER_CMAKE_CXX_COMPILER_ID;
				ctx["OSU_TRACKER_CMAKE_CXX_COMPILER_VERSION"] = OSU_TRACKER_CMAKE_CXX_COMPILER_VERSION;
				ctx["OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID"] = OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID;
				ctx["OSU_TRACKER_CMAKE_CXX_COMPILER"] = OSU_TRACKER_CMAKE_CXX_COMPILER;
				ctx["OSU_TRACKER_CMAKE_SYSTEM_NAME"] = OSU_TRACKER_CMAKE_SYSTEM_NAME;

				auto page = crow::mustache::load("info.html").render(ctx);
				return page;
			});

			CROW_ROUTE(app, "/help")([](){
				crow::mustache::context ctx;
				ctx["title"] = OSU_TRACKER_NAME;
				ctx["version"] = OSU_TRACKER_VERSION;

				ctx["tracker_config_name"] = "Help";

				std::vector<crow::json::wvalue> fields;

				for (const auto& row : config::data::arr) {
					crow::json::wvalue el;
					el["name"] = row.name;
					el["key"] = row.key;
					std::string servers;
					if (row.banchoSupport) servers += "Bancho";
					if (row.titanicSupport) {
						if (!servers.empty()) servers += ", ";
						servers += "Titanic";
					}
					el["server"] = servers;
					fields.push_back(std::move(el));
				}

				ctx["fields"] = std::move(fields);

				auto page = crow::mustache::load("help.html").render(ctx);
				return page;
			});

			CROW_ROUTE(app, "/template/default")([](crow::SimpleApp app) {
				crow::mustache::context ctx;
				ctx["hostname"] = OSU_TRACKER_WEBSERVER_IP;
				ctx["port"] = OSU_TRACKER_WEBSERVER_PORT;

				auto page = crow::mustache::load("template/default.html").render(ctx);
				return page;
			});

			app.add_static_dir();

		} // end of skip

		console::writeLog("Starting Web Server...", true);
		console::writeLog("Web Server should be accessible under:", true);
		console::writeLog("#####################", true, 255, 255, 0);
		std::string url = "http://" + (std::string)OSU_TRACKER_WEBSERVER_IP + ":" + std::to_string(OSU_TRACKER_WEBSERVER_PORT);
		console::writeLog(url, true, 0, 140, 255);
		console::writeLog("#####################", true, 255, 255, 0);
		app.bindaddr(OSU_TRACKER_WEBSERVER_IP).port(OSU_TRACKER_WEBSERVER_PORT).signal_clear().run(); // blocking
		console::writeLog("Web Server Terminated", true , 255, 0, 0);
		return instance().shutdown_webServer;
	}
};
