#pragma once
// Pages
#include <Controller/Web/Routes/Pages/IndexRoute.h>
#include <Controller/Web/Routes/Pages/SettingsRoute.h>
#include <Controller/Web/Routes/Pages/InfoRoute.h>
#include <Controller/Web/Routes/Pages/HelpRoute.h>
#include <Controller/Web/Routes/Pages/TemplateRoute.h>
// WebSockets
#include <Controller/Web/Routes/WebSockets/MainWsRoute.h>
#include <Controller/Web/Routes/WebSockets/OverlayDataWsRoute.h>

namespace Web::RouteRegister {

	inline void registerAll(crow::SimpleApp& app) {
		// Pages
		Web::Routes::Pages::registerIndex(app);
		Web::Routes::Pages::registerSettings(app);
		Web::Routes::Pages::registerInfo(app);
		Web::Routes::Pages::registerHelp(app);
		Web::Routes::Pages::registerTemplate(app);

		// WebSockets
		Web::Routes::WebSockets::registerMain(app);
		Web::Routes::WebSockets::registerOverlayData(app);
	}
}
