#pragma once
#include <algorithm>
#include <crow.h>
#include <Controller/Core/config.h>
#include <Controller/Web/Layout.h>

namespace Web::Routes::Pages {

	inline void registerSettings(crow::SimpleApp& app) {
		CROW_ROUTE(app, "/settings")([]() {
			crow::mustache::context ctx;

			ctx["title"]    = OSU_TRACKER_NAME;
			ctx["version"]  = OSU_TRACKER_VERSION;
			ctx["hostname"] = OSU_TRACKER_WEBSERVER_HOST;
			ctx["port"]     = OSU_TRACKER_WEBSERVER_PORT;
			ctx["nav_settings_active"] = "active";

			ctx["bancho_id_name"] = "Bancho User ID";
			ctx["bancho_id_val"]  = config::application.get("banchoId");
			ctx["bancho_id_desc"] = "Your osu! Bancho user ID.";

			ctx["titanic_id_name"] = "Titanic User ID";
			ctx["titanic_id_val"]  = config::application.get("titanicId");
			ctx["titanic_id_desc"] = "Your Titanic private server user ID.";

			ctx["client_id_name"] = "Client ID";
			ctx["client_id_val"]  = config::application.get("clientId");
			ctx["client_id_desc"] = "osu! API V2 Client ID.";

			ctx["client_secret_name"] = "Client Secret";
			ctx["client_secret_val"]  = config::application.get("clientSecret");
			ctx["client_secret_desc"] = "osu! API V2 Client Secret ( DO NOT SHARE )!";

			ctx["api_refreshInterval_name"] = "API Refresh Interval";
			ctx["api_refreshInterval_val"]  = config::application.get("apiInterval");
			ctx["api_refreshInterval_desc"] = "Time in (ms) till api fetches again in the loop.";

			ctx["gameMode_name"] = "Game Mode";
			ctx["gameMode_val_" + std::to_string(static_cast<int>(config::application.gameMode))] = "selected";
			ctx["gameMode_desc"] = "Game Mode to track.";

			ctx["server_name"] = "Server";
			ctx["server_val_" + std::to_string(static_cast<int>(config::application.server))] = "selected";
			ctx["server_desc"] = "Which Server you want to track, bancho or a private server.";

			switch (config::application.server) {
				case config::server::titanic:
					ctx["hide_on_privateServer"] = "d-none";
					break;
				default: break;
			}

			std::vector<crow::json::wvalue> elements;

			// Show every entry sorted by its current sort value.
			// Badges in the template indicate which server(s) each field supports.
			std::vector<const config::dataEntry*> sorted;
			for (const config::dataEntry& e : config::data::arr)
				sorted.push_back(&e);
			std::sort(sorted.begin(), sorted.end(),
				[](const config::dataEntry* a, const config::dataEntry* b) { return a->sort < b->sort; });

			switch (config::application.server) {
				case config::server::bancho:  ctx["tracker_config_name"] = "Bancho Tracker Config";  break;
				case config::server::titanic: ctx["tracker_config_name"] = "Titanic Tracker Config"; break;
			}

			for (const config::dataEntry* e : sorted) {
				crow::json::wvalue el;
				el["id"]    = e->key;
				el["label"] = e->name;
				el["sort"]  = e->sort;
				if (e->display)        el["checked"] = " checked";
				if (e->banchoSupport)  el["bancho"]  = true;
				if (e->titanicSupport) el["titanic"] = true;
				elements.push_back(std::move(el));
			}
			ctx["trackerConfig"] = std::move(elements);

			return Web::Layout::render("settings.html", ctx);
		});
	}
}

