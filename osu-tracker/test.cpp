#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <json.hpp>
#include <cpr/cpr.h>
#include "../header/ext.h"
#include "../header/api.h"
#include "../header/console.h"
#include "../header/config.h"
#include "../header/webserver.h"
#include "../header/ui/ui.h"

TEST_CASE("TScore to Level") {
    long double level = api::getLevelFromScore(101);
    CAPTURE(level);
    REQUIRE(level == 0);
}
