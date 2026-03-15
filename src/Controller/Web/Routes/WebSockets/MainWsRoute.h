#pragma once
#include <crow.h>
#include <json.hpp>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <Controller/Web/WebState.h>
#include <Controller/Web/ToastHandler.h>
#include <Controller/Core/config.h>
#include <Controller/ui.h>

namespace Web::Routes::WebSockets {

	inline void registerMain(crow::SimpleApp& app) {
		CROW_WEBSOCKET_ROUTE(app, "/ws/main/")
		.onopen([](crow::websocket::connection& conn) {
			std::lock_guard<std::mutex> _(Web::WebHandler::ws_mutex);
			Web::WebHandler::clients.insert(&conn);
		})
		.onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
			std::lock_guard<std::mutex> _(Web::WebHandler::ws_mutex);
			Web::WebHandler::clients.erase(&conn);
		})
		.onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
			std::lock_guard<std::mutex> _(Web::WebHandler::ws_mutex);
			nlohmann::json j = nlohmann::json::parse(data);
			std::string cmd = j["cmd"];
			console::writeLog("Command received: " + cmd);

			if (cmd[0] == '#') {
				if (cmd == "#restart")  Web::WebHandler::shutdown(true);
				if (cmd == "#shutdown") Web::WebHandler::shutdown(false);
				if (cmd == "#open_ui")  Web::WebHandler::startUiThread();
				if (cmd == "#count") {
					Web::WebHandler::counter++;
					conn.send_text(Web::ToastHandler::sendToast(std::to_string(Web::WebHandler::counter)).dump());
				}
				if (cmd == "#update") {
					if (Web::WebHandler::performUpdateCheck() == 0)
						Web::WebHandler::shutdown(false);
					conn.send_text(Web::ToastHandler::sendToast((std::string)"No updates found.").dump());
				}
				if (cmd == "#resetSession") {
					conn.send_text(Web::ToastHandler::sendToast("Resetting session...").dump());
					std::thread([] {
						ui::stopFetchThread();
						ui::startFetchThread(true);
					}).detach();
				}
				if (cmd == "#resetSettings") {
					config::resetToDefaults();
					config::remove();
					Web::WebHandler::shutdown(true);
				}
				if (cmd == "#saveSettings") {
					// Build a key→value map from the incoming payload
					std::unordered_map<std::string, std::string> incoming;
					for (const auto& item : j["msg"]["applicationConfig"])
						incoming[item["key"].get<std::string>()] = item["value"].get<std::string>();

					// Restart session if any of these change
					bool resetSession = false;
					for (const std::string& k : { "banchoId", "titanicId", "clientId", "clientSecret", "gameMode", "server" }) {
						if (incoming.count(k) && incoming.at(k) != config::application.get(k)) {
							resetSession = true;
							break;
						}
					}

					for (const auto& [key, val] : incoming)
						config::application.set(key, val);
					{
						std::unique_lock<std::shared_mutex> dataLock(config::data::dataMutex);
						for (const auto& item : j["msg"]["trackerConfig"]) {
							int idx = config::data::getIndex(item["key"].get<std::string>());
							if (idx < 0) continue;
							if (item.contains("display")) config::data::arr[idx].display = item["display"].get<bool>();
							if (item.contains("sort"))    config::data::arr[idx].sort    = item["sort"].get<int>();
						}
						// Immediately sync arrFormatted so the ImGui UI reflects
						// toggle/sort changes without waiting for the next API poll.
						ui::updateFormat();
					}
					config::write();
					if (resetSession) {
						conn.send_text(Web::ToastHandler::sendToast("Settings saved. Resetting session...").dump());
						std::thread([] {
							ui::stopFetchThread();
							ui::startFetchThread(true);
						}).detach();
					} else {
						conn.send_text(Web::ToastHandler::sendToast("Settings saved.").dump());
					}
				}
			}
		});
	}
}

