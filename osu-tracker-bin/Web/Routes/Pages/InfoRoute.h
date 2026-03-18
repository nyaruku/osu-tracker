#pragma once
#include <crow.h>
#include <Core/Config.h>
#include <Web/Layout.h>
#include <curl/curl.h>

namespace Web::Routes::Pages {

	inline void registerInfo(crow::SimpleApp& app) {
		CROW_ROUTE(app, "/info")([](crow::SimpleApp app) {
			crow::mustache::context ctx;
			ctx["title"]    = OSU_TRACKER_NAME;
			ctx["version"]  = OSU_TRACKER_VERSION;
			ctx["hostname"] = OSU_TRACKER_WEBSERVER_HOST;
			ctx["port"]     = OSU_TRACKER_WEBSERVER_PORT;
			ctx["nav_info_active"] = "active";

			// project
			ctx["thread_count"] = std::to_string(app.concurrency());
			ctx["github"]       = "https://github.com/nyaruku/osu-tracker";
			ctx["creator"]      = "Railgun";
			ctx["profile"]      = "https://osu.ppy.sh/users/13817114";
			ctx["discord"]      = "https://discord.gg/QNCmZBqwBQ";

			// build info
			ctx["build_type"]   = OSU_TRACKER_CMAKE_BUILD_TYPE;
			ctx["release_type"] = OSU_TRACKER_RELEASE_TYPE;

			// cmake info
			ctx["OSU_TRACKER_CMAKE_GENERATOR"]                    = OSU_TRACKER_CMAKE_GENERATOR;
			ctx["OSU_TRACKER_CMAKE_CXX_STANDARD"]                 = OSU_TRACKER_CMAKE_CXX_STANDARD;
			ctx["OSU_TRACKER_CMAKE_VERSION"]                      = OSU_TRACKER_CMAKE_VERSION;
			ctx["OSU_TRACKER_CMAKE_MINIMUM_REQUIRED_VERSION"]     = OSU_TRACKER_CMAKE_MINIMUM_REQUIRED_VERSION;
			ctx["OSU_TRACKER_CMAKE_CXX_COMPILER_ID"]              = OSU_TRACKER_CMAKE_CXX_COMPILER_ID;
			ctx["OSU_TRACKER_CMAKE_CXX_COMPILER_VERSION"]         = OSU_TRACKER_CMAKE_CXX_COMPILER_VERSION;
			ctx["OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID"] = OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID;
			ctx["OSU_TRACKER_CMAKE_CXX_COMPILER"]                 = OSU_TRACKER_CMAKE_CXX_COMPILER;
			ctx["OSU_TRACKER_CMAKE_SYSTEM_NAME"]                  = OSU_TRACKER_CMAKE_SYSTEM_NAME;

			return Web::Layout::render("info.html", ctx);
		});
	}
}

