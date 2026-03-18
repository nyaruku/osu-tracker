#pragma once
#include <crow.h>

namespace Web::Layout {

	inline crow::response render(const std::string& page, crow::mustache::context& ctx) {

		ctx["route_content"] = crow::mustache::load(page).render(ctx).body_;
		return crow::mustache::load("base.html").render(ctx);
	}
}
