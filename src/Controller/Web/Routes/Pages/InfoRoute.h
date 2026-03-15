#pragma once
#include <crow.h>
#include <Controller/Core/config.h>
#include <Controller/Web/Layout.h>
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
			ctx["discord"]      = "https://discord.gg/amj5vBjTQU";

			// build info
			ctx["build_type"]   = OSU_TRACKER_CMAKE_BUILD_TYPE;
			ctx["release_type"] = OSU_TRACKER_RELEASE_TYPE;

			// libraries
			ctx["crow_version"]          = crow::VERSION;
			ctx["crow_repo_url"]         = "https://github.com/CrowCpp/Crow/tree/e8b125159ec5e61384187e36bbf6d14f1fad504f";
			ctx["libcurl_version"]       = LIBCURL_VERSION;
			ctx["libcurl_repo_url"]      = "https://github.com/curl/curl";
			ctx["asio_version"]          = "10.30.02";
			ctx["asio_repo_url"]         = "https://github.com/chriskohlhoff/asio";
			ctx["cpr_version"]           = CPR_VERSION;
			ctx["cpr_repo_url"]          = "https://github.com/libcpr/cpr";
			ctx["nlohmannJson_version"]  = "3.11.2";
			ctx["nlohmannJson_repo_url"] = "https://github.com/nlohmann/json";

			const char* ZLIB_VERSION;
			curl_version_info_data* version_info = curl_version_info(CURLVERSION_NOW);
			ZLIB_VERSION = (version_info->features & CURL_VERSION_LIBZ) ? version_info->libz_version : "Unknown";
			ctx["zlib_version"]  = ZLIB_VERSION;
			ctx["zlib_repo_url"] = "https://github.com/madler/zlib";

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

