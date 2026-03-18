#pragma once
#include <crow.h>
#include <Core/Config.h>
#include <Web/Layout.h>
#include <Web/TemplateRegister.h>

namespace Web::Routes::Pages {

	inline void registerIndex(crow::SimpleApp& app) {
		CROW_ROUTE(app, "/")([]() {
			crow::mustache::context ctx;
			ctx["title"]    = OSU_TRACKER_NAME;
			ctx["version"]  = OSU_TRACKER_VERSION;
			ctx["hostname"] = OSU_TRACKER_WEBSERVER_HOST;
			ctx["port"]     = OSU_TRACKER_WEBSERVER_PORT;
			ctx["nav_index_active"] = "active";

			std::vector<crow::json::wvalue> overlays;
			for (const auto& t : Web::TemplateRegister::registry) {
				crow::json::wvalue el;
				el["title"]    = t.title;
				el["filename"] = t.filename;
				el["route"]    = t.route;
				el["creator"]  = t.creator;
				overlays.push_back(std::move(el));
			}
			ctx["overlays"] = std::move(overlays);

			return Web::Layout::render("index.html", ctx);
		});
	}
}
