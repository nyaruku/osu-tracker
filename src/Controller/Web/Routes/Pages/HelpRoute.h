#pragma once
#include <crow.h>
#include <Controller/Core/config.h>
#include <Controller/Web/Layout.h>

namespace Web::Routes::Pages {

	inline void registerHelp(crow::SimpleApp& app) {
		CROW_ROUTE(app, "/help")([]() {
			crow::mustache::context ctx;
			ctx["title"]               = OSU_TRACKER_NAME;
			ctx["version"]             = OSU_TRACKER_VERSION;
			ctx["hostname"]            = OSU_TRACKER_WEBSERVER_HOST;
			ctx["port"]                = OSU_TRACKER_WEBSERVER_PORT;
			ctx["nav_help_active"]     = "active";
			ctx["tracker_config_name"] = "Help";

			std::vector<crow::json::wvalue> fields;
			for (const auto& row : config::data::arr) {
				crow::json::wvalue el;
				el["name"] = row.name;
				el["key"]  = row.key;
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

			return Web::Layout::render("help.html", ctx);
		});
	}
}

