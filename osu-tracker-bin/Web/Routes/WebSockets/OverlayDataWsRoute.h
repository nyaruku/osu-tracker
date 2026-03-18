#pragma once
#include <crow.h>
#include <json.hpp>
#include <shared_mutex>
#include <unordered_map>
#include <Web/WebState.h>
#include <Core/Config.h>

namespace Web::Routes::WebSockets {

	inline void registerOverlayData(crow::SimpleApp& app) {
		CROW_WEBSOCKET_ROUTE(app, "/ws/overlay-data/")
		.onopen([](crow::websocket::connection& conn) {
			std::lock_guard<std::mutex> _(Web::WebHandler::ws_mutex);
			Web::WebHandler::clients.insert(&conn);

			nlohmann::json j;
			{
				std::shared_lock<std::shared_mutex> dataLock(Core::Config::data::dataMutex);

				j["username"] = Core::Config::user.username;

				std::unordered_map<std::string, const Core::Config::dataEntry*> fmtMap;
				for (const auto& f : Core::Config::data::arrFormatted)
					fmtMap[f.key] = &f;

				for (const auto& row : Core::Config::data::arr) {
					j[row.key + "_init_raw"]    = row.init;
					j[row.key + "_change_raw"]  = row.change;
					j[row.key + "_current_raw"] = row.current;

					auto it = fmtMap.find(row.key);
					if (it != fmtMap.end()) {
						j[row.key + "_init"]    = it->second->init;
						j[row.key + "_change"]  = it->second->change;
						j[row.key + "_current"] = it->second->current;
					}
				}
			}
			conn.send_text(j.dump());
		})
		.onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
			std::lock_guard<std::mutex> _(Web::WebHandler::ws_mutex);
			Web::WebHandler::clients.erase(&conn);
		})
		.onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
			std::lock_guard<std::mutex> _(Web::WebHandler::ws_mutex);
			try {
				nlohmann::json stats;
				{
					std::shared_lock<std::shared_mutex> dataLock(Core::Config::data::dataMutex);

					stats["username"] = Core::Config::user.username;

					std::unordered_map<std::string, const Core::Config::dataEntry*> fmtMap;
					for (const auto& f : Core::Config::data::arrFormatted)
						fmtMap[f.key] = &f;

					for (const auto& row : Core::Config::data::arr) {
						stats[row.key + "_init_raw"]    = row.init;
						stats[row.key + "_change_raw"]  = row.change;
						stats[row.key + "_current_raw"] = row.current;

						auto it = fmtMap.find(row.key);
						if (it != fmtMap.end()) {
							stats[row.key + "_init"]    = it->second->init;
							stats[row.key + "_change"]  = it->second->change;
							stats[row.key + "_current"] = it->second->current;
						}
					}
				}
				conn.send_text(stats.dump());
			}
			catch (const std::exception& e) {
				Core::Console::writeLog("Overlay WS parse error: " + std::string(e.what()), true, 255, 0, 0);
			}
		});
	}
}

