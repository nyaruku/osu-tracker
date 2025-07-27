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
    long double scoreNegative = api::getLevelFromScore(-10000000);
    CAPTURE(scoreNegative);
    REQUIRE(scoreNegative == 1);

    long double level = api::getLevelFromScore(0);
    CAPTURE(level);
    REQUIRE(level == 1);

    long double level1 = api::getLevelFromScore(1);
    CAPTURE(level1);
    REQUIRE(level1 == 1);

    long double level2 = api::getLevelFromScore(30000);
    CAPTURE(level2);
    REQUIRE(level2 == 2);
    
    long double level15 = api::getLevelFromScore(21350000);
    CAPTURE(level15);
    REQUIRE(level15 == 15);

    long double level100 = api::getLevelFromScore(26931190828);
    CAPTURE(level100);
    REQUIRE(level100 == 100);

    long double level101 = api::getLevelFromScore(126931190828);
    CAPTURE(level101);
    REQUIRE(level101 == 101);
}
