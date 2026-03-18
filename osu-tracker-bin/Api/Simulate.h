#pragma once
#include <Api/Helpers.h>
#include <Core/Config.h>

namespace Api::Simulate {

    static void simulate(bool init) {
        if (!init) return;
        Core::Config::user.username = "Player";
        Helpers::setField("level",       "100",          true);
        Helpers::setField("scoreRank",   "100",          true);
        Helpers::setField("ppRank",      "50000",        true);
        Helpers::setField("ppv1Rank",    "50000",        true);
        Helpers::setField("countryRank", "2000",         true);
        Helpers::setField("pp",          "1000",         true);
        Helpers::setField("ppv1",        "1000",         true);
        Helpers::setField("acc",         "99",           true);
        Helpers::setField("playtime",    "100000",       true);
        Helpers::setField("playcount",   "25000",        true);
        Helpers::setField("maxCombo",    "5000",         true);
        Helpers::setField("totalHits",   "5000000",      true);
        Helpers::setField("replayViews", "1000",         true);
        Helpers::setField("rankedScore", "10000000000",  true);
        Helpers::setField("totalScore",  "10000000000",  true);
        Helpers::setField("clears",      "10000",        true);
        Helpers::setField("totalClears", "15000",        true);
        Helpers::setField("silverSS",    "1000",         true);
        Helpers::setField("goldSS",      "1000",         true);
        Helpers::setField("silverS",     "1000",         true);
        Helpers::setField("goldS",       "1000",         true);
        Helpers::setField("a",           "1000",         true);
        Helpers::setField("b",           "1000",         true);
        Helpers::setField("c",           "1000",         true);
        Helpers::setField("d",           "1000",         true);
        Helpers::setField("totalSS",     "2000",         true);
        Helpers::setField("totalS",      "2000",         true);
        Helpers::setField("targetRank",  "99",           true);
        Helpers::setField("targetUser",  "OtherPlayer",  true);
        Helpers::setField("targetScore", "20000000000",  true);
        Helpers::setField("completion",  "50.00",        true);
        // static differences so the UI shows non-zero change values
        Helpers::setField("level",       "100.5");
        Helpers::setField("scoreRank",   "50");
        Helpers::setField("ppRank",      "20000");
        Helpers::setField("ppv1Rank",    "20000");
        Helpers::setField("countryRank", "500");
        Helpers::setField("pp",          "2000");
        Helpers::setField("ppv1",        "2000");
        Helpers::setField("acc",         "99.5");
        Helpers::setField("playtime",    "200000");
        Helpers::setField("playcount",   "35000");
        Helpers::setField("maxCombo",    "6000");
        Helpers::setField("totalHits",   "6000000");
        Helpers::setField("replayViews", "2000");
        Helpers::setField("rankedScore", "20000000000");
        Helpers::setField("totalScore",  "20000000000");
        Helpers::setField("clears",      "20000");
        Helpers::setField("totalClears", "20000");
        Helpers::setField("silverSS",    "2000");
        Helpers::setField("goldSS",      "2000");
        Helpers::setField("silverS",     "2000");
        Helpers::setField("goldS",       "2000");
        Helpers::setField("a",           "2000");
        Helpers::setField("b",           "2000");
        Helpers::setField("c",           "2000");
        Helpers::setField("d",           "2000");
        Helpers::setField("totalSS",     "3000");
        Helpers::setField("totalS",      "3000");
        Helpers::setField("targetRank",  "99.5");
        Helpers::setField("targetUser",  "OtherPlayer");
        Helpers::setField("targetScore", "30000000000");
        Helpers::setField("completion",  "51.00");
        Helpers::calcDifference();
    }

}