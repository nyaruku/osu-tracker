#pragma once
#include <crow.h>

namespace Web::Layout {

	/*
		Renders a page fragment into base.html.
		The caller fills ctx with all page-specific variables.
		Layout variables expected by base.html (title, version, hostname, port)
		must also be present in ctx before calling this.
	*/
	inline crow::response render(const std::string& page, crow::mustache::context& ctx) {

		ctx["route_content"] = crow::mustache::load(page).render(ctx).body_;
		return crow::mustache::load("base.html").render(ctx);
	}
}
