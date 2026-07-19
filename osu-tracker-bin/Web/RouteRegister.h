#pragma once
// Pages
#include <Web/Routes/Pages/IndexRoute.h>
#include <Web/Routes/Pages/SettingsRoute.h>
#include <Web/Routes/Pages/InfoRoute.h>
#include <Web/Routes/Pages/HelpRoute.h>
#include <Web/Routes/Pages/TemplateRoute.h>
// WebSockets
#include <Web/Routes/WebSockets/MainWsRoute.h>
#include <Web/Routes/WebSockets/OverlayDataWsRoute.h>

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