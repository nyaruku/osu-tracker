#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <regex>
#include <algorithm>
#include <crow.h>
#include <Core/Config.h>
#include <Core/Console.h>

namespace Web::TemplateRegister {

	struct TemplateInfo {
		std::string filename;
		std::string stem;
		std::string route;
		std::string title;
		std::string creator;
	};

	inline std::vector<TemplateInfo> registry;

	static std::string extractMeta(const std::string& html, const std::string& name) {
		std::regex regex1("<meta[^>]+name=\"" + name + "\"[^>]+content=\"([^\"]*)\"");
		std::regex regex2("<meta[^>]+content=\"([^\"]*)\"[^>]+name=\"" + name + "\"");
		std::smatch m;
		if (std::regex_search(html, m, regex1)) return m[1].str();
		if (std::regex_search(html, m, regex2)) return m[1].str();
		return {};
	}

	inline void scan(const std::string& dir = "./www/template/") {
		registry.clear();

		if (!std::filesystem::exists(dir)) {
			Core::Console::writeLog("TemplateRegistry: directory not found: " + dir, true, 255, 165, 0);
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			if (entry.path().extension() != ".html") continue;

			std::ifstream f(entry.path());
			if (!f.is_open()) continue;

			std::string head(4096, '\0');
			f.read(head.data(), 4096);
			head.resize(static_cast<std::size_t>(f.gcount()));

			if (extractMeta(head, "type") != "overlay") continue;

			std::string title   = extractMeta(head, "title");
			std::string creator = extractMeta(head, "creator");
			if (title.empty() || creator.empty()) continue;

			std::string filename = entry.path().filename().string();
			std::string stem     = entry.path().stem().string();

			registry.push_back({ filename, stem, "/template/" + stem, title, creator });
			Core::Console::writeLog("TemplateRegistry: registered template \"" + title + "\" -> /template/" + stem, false);
		}

		std::sort(registry.begin(), registry.end(),
			[](const TemplateInfo& a, const TemplateInfo& b) { return a.title < b.title; });
	}

	inline void registerRoutes(crow::SimpleApp& app) {
		CROW_ROUTE(app, "/template/<string>")([](const std::string& name) -> crow::response {
			for (const auto& t : registry) {
				if (t.stem == name) {
					crow::mustache::context ctx;
					ctx["hostname"] = OSU_TRACKER_WEBSERVER_HOST;
					ctx["port"]     = OSU_TRACKER_WEBSERVER_PORT;
					return crow::mustache::load("template/" + t.filename).render(ctx);
				}
			}
			return crow::response(404, "Template not found: " + name);
		});
	}
}