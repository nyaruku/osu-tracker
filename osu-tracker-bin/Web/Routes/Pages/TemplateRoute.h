#pragma once
#include <crow.h>
#include <Web/TemplateRegister.h>

namespace Web::Routes::Pages {
	inline void registerTemplate(crow::SimpleApp& app) {
		Web::TemplateRegister::scan();
		Web::TemplateRegister::registerRoutes(app);
	}
}