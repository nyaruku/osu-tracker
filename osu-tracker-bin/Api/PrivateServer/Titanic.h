#pragma once
#include <mutex>
#include <thread>
#include <json.hpp>
#include <cpr/cpr.h>
#include <Api/Status.h>
#include <Api/Helpers.h>
#include <Core/Config.h>
#include <Core/Console.h>
#include <Core/Helpers.h>

namespace Api::PrivateServer::Titanic {

    static int titanic(bool init) {
        Core::Console::writeLog(std::string("titanic_api(init) -> ") + Core::Helpers::bool2str(init), false, 0, 0, 255);
        try {
            cpr::Response r_users, r_stats;

            std::thread t1([&]() {
                r_users = cpr::Get(
                    cpr::Url{ "https://api.titanic.sh/users/" + std::to_string(Core::Config::application.activeId()) },
                    cpr::Header{ { "Content-Type", "application/json" }, { "Accept", "application/json" } },
                    cpr::Timeout{ Status::TIMEOUT_MS }
                );
                Core::Console::writeLog("titanic_api(/users) -> Status: " + std::to_string(r_users.status_code), false, 0, 255, 0);
            });
            std::thread t2([&]() {
                r_stats = cpr::Get(
                    cpr::Url{ "https://api.titanic.sh/stats" },
                    cpr::Header{ { "Content-Type", "application/json" }, { "Accept", "application/json" } },
                    cpr::Timeout{ Status::TIMEOUT_MS }
                );
                Core::Console::writeLog("titanic_api(/stats) -> Status: " + std::to_string(r_stats.status_code), false, 0, 255, 0);
            });
            t1.join(); t2.join();

            Status::statusTitanic = { r_users.status_code, true, std::chrono::system_clock::now() };

            nlohmann::json _j  = nlohmann::json::parse(r_users.text);
            nlohmann::json _j2 = nlohmann::json::parse(r_stats.text);

            const int         modeIdx = static_cast<int>(Core::Config::application.gameMode);
            const std::string modeStr = std::to_string(modeIdx);
            const int totalMaps =
                _j2["beatmap_modes"][modeStr]["count_ranked"].get<int>()    +
                _j2["beatmap_modes"][modeStr]["count_approved"].get<int>()  +
                _j2["beatmap_modes"][modeStr]["count_qualified"].get<int>() +
                _j2["beatmap_modes"][modeStr]["count_loved"].get<int>();
            const int completionDenom = (modeIdx == 0) ? totalMaps : totalMaps +
                _j2["beatmap_modes"]["0"]["count_ranked"].get<int>()    +
                _j2["beatmap_modes"]["0"]["count_approved"].get<int>()  +
                _j2["beatmap_modes"]["0"]["count_qualified"].get<int>() +
                _j2["beatmap_modes"]["0"]["count_loved"].get<int>();

            Core::Config::user.username = _j["name"].get<std::string>();

            // avatar
            {
                std::string url = _j.value("avatar_url",
                    "https://a.titanic.sh/" + std::to_string(Core::Config::application.activeId()));
                if (url != Core::Config::user.avatarUrl) {
                    Core::Config::user.avatarUrl = url;
                    cpr::Response av = cpr::Get(cpr::Url{ url }, cpr::Timeout{ Status::TIMEOUT_MS });
                    if (av.status_code == 200) {
                        std::lock_guard<std::mutex> lk(Core::Config::user.avatarMutex);
                        Core::Config::user.avatarBytes.assign(av.text.begin(), av.text.end());
                        Core::Config::user.avatarDirty = true;
                    }
                }
            }

            const auto& s = _j["stats"][modeIdx];
            Helpers::setField("level",       std::to_string(Core::Helpers::getLevelFromScore(s["tscore"].get<long long>())), init);
            Helpers::setField("rankedScore", std::to_string(s["rscore"].get<long long>()),    init);
            Helpers::setField("totalScore",  std::to_string(s["tscore"].get<long long>()),    init);
            Helpers::setField("ppRank",      std::to_string(s["rank"].get<int>()),            init);
            Helpers::setField("pp",          std::to_string(s["pp"].get<float>()),            init);
            Helpers::setField("ppv1",        std::to_string(s["ppv1"].get<float>()),          init);
            Helpers::setField("acc",         std::to_string(s["acc"].get<float>() * 100),     init);
            Helpers::setField("playtime",    std::to_string(s["playtime"].get<long long>()),  init);
            Helpers::setField("playcount",   std::to_string(s["playcount"].get<int>()),       init);
            Helpers::setField("totalHits",   std::to_string(s["total_hits"].get<long long>()), init);
            Helpers::setField("silverSS",    std::to_string(s["xh_count"].get<int>()),        init);
            Helpers::setField("goldSS",      std::to_string(s["x_count"].get<int>()),         init);
            Helpers::setField("silverS",     std::to_string(s["sh_count"].get<int>()),        init);
            Helpers::setField("goldS",       std::to_string(s["s_count"].get<int>()),         init);
            Helpers::setField("a",           std::to_string(s["a_count"].get<int>()),         init);
            Helpers::setField("b",           std::to_string(s["b_count"].get<int>()),         init);
            Helpers::setField("c",           std::to_string(s["c_count"].get<int>()),         init);
            Helpers::setField("d",           std::to_string(s["d_count"].get<int>()),         init);
            Helpers::setField("maxCombo",    std::to_string(s["max_combo"].get<int>()),       init);
            Helpers::setField("replayViews", std::to_string(s["replay_views"].get<int>()),    init);

            const auto& rnk = _j["rankings"][modeStr];
            Helpers::setField("countryRank", std::to_string(rnk["performance"]["country"].get<int>()), init);
            Helpers::setField("ppv1Rank",    std::to_string(rnk["ppv1"]["global"].get<int>()),         init);
            Helpers::setField("scoreRank",   std::to_string(rnk["rscore"]["global"].get<int>()),       init);

            Helpers::setField("totalSS",     std::to_string(Helpers::getIntField("silverSS") + Helpers::getIntField("goldSS")), init);
            Helpers::setField("totalS",      std::to_string(Helpers::getIntField("silverS")  + Helpers::getIntField("goldS")),  init);

            Helpers::setField("clears",      std::to_string(Helpers::getIntField("totalSS")  + Helpers::getIntField("totalS") + Helpers::getIntField("a")), init);
            Helpers::setField("totalClears", std::to_string(Helpers::getIntField("clears") + Helpers::getIntField("b") + Helpers::getIntField("c") + Helpers::getIntField("d")), init);

            Helpers::setField("completion",  std::to_string((static_cast<float>(Helpers::getIntField("totalClears")) / static_cast<float>(completionDenom)) * 100), init);

            Helpers::calcDifference();
            return r_users.status_code;
        }
        catch (const nlohmann::json::exception& e) { Core::Console::writeLog("titanic_api() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
        catch (const cpr::Error& e)                { Core::Console::writeLog("titanic_api() -> CPR: "  + e.message,             true, 255, 0, 0); }
        catch (const std::exception& e)            { Core::Console::writeLog("titanic_api() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
        catch (...)                                { Core::Console::writeLog("titanic_api() -> Unknown",                        true, 255, 0, 0); }
        return -1;
    }
}