#pragma once
#include <array>
#include <mutex>
#include <thread>
#include <json.hpp>
#include <cpr/cpr.h>
#include <Api/Status.h>
#include <Api/Helpers.h>
#include <Core/Config.h>
#include <Core/Console.h>
#include <Core/Helpers.h>

namespace Api::Bancho {

    inline std::string auth_token;

    static int api_auth() {
        try {
            const std::string body =
                R"({"client_id":)"       + std::to_string(Core::Config::application.clientId) +
                R"(, "client_secret":")" + Core::Config::application.clientSecret +
                R"(", "grant_type":"client_credentials", "scope":"public"})";

            cpr::Response r = cpr::Post(
                cpr::Url{ "https://osu.ppy.sh/oauth/token" },
                cpr::Body{ body },
                cpr::Header{ { "Content-Type", "application/json" } },
                cpr::Timeout{Status::TIMEOUT_MS }
            );

            nlohmann::json response = nlohmann::json::parse(r.text);
            auth_token = "Bearer " + response.value("access_token", "");
            Core::Console::writeLog("api_auth() -> Status: " + std::to_string(r.status_code), false, 0, 255, 0);
            Status::statusAuth = { r.status_code, true, std::chrono::system_clock::now() };
            return r.status_code;
        }
        catch (const nlohmann::json::exception& e) { Core::Console::writeLog("api_auth() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
        catch (const cpr::Error& e)                { Core::Console::writeLog("api_auth() -> CPR: "  + e.message,             true, 255, 0, 0); }
        catch (const std::exception& e)            { Core::Console::writeLog("api_auth() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
        catch (...)                                { Core::Console::writeLog("api_auth() -> Unknown",                        true, 255, 0, 0); }
        return -1;
    }

    static int api(bool init) {
        try {
            static constexpr std::array<const char*, 4> modeNames = { "osu", "taiko", "fruits", "mania" };
            const std::string mode = modeNames[static_cast<int>(Core::Config::application.gameMode)];

            cpr::Response r = cpr::Get(
                cpr::Url{ "https://osu.ppy.sh/api/v2/users/" + std::to_string(Core::Config::application.activeId()) + "/" + mode + "?key=id" },
                cpr::Header{
                    { "Content-Type",  "application/json" },
                    { "Accept",        "application/json" },
                    { "Authorization", auth_token }
                },
                cpr::Timeout{ Status::TIMEOUT_MS }
            );
            Core::Console::writeLog("api() -> Status: " + std::to_string(r.status_code), false, 0, 255, 0);
            Status::statusBancho = { r.status_code, true, std::chrono::system_clock::now() };
            if (r.status_code != 200) return r.status_code;

            nlohmann::json _j = nlohmann::json::parse(r.text);
            if (!_j.contains("statistics") || _j["statistics"].is_null()) return r.status_code;
            const auto& st = _j.at("statistics");
            const auto& gc = st.at("grade_counts");
            Core::Config::user.username = _j["username"].get<std::string>();

            // avatar
            {
                std::string url = _j.value("avatar_url", "https://a.ppy.sh/" + std::to_string(Core::Config::application.activeId()));
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

            // Bancho returns null (not 0) for rank/pp/etc. when a user has no plays in a mode.
            auto nsInt = [&](const nlohmann::json& j, const char* k, int d = 0) {
                const auto it = j.find(k);
                return (it != j.end() && !it->is_null()) ? it->get<int>() : d;
            };
            auto nsFloat = [&](const nlohmann::json& j, const char* k, float d = 0.f) {
                const auto it = j.find(k);
                return (it != j.end() && !it->is_null()) ? it->get<float>() : d;
            };
            auto nsLL = [&](const nlohmann::json& j, const char* k, long long d = 0LL) {
                const auto it = j.find(k);
                return (it != j.end() && !it->is_null()) ? it->get<long long>() : d;
            };

            Helpers::setField("level",       std::to_string(Core::Helpers::getLevelFromScore(nsLL  (st, "total_score"))),  init);
            Helpers::setField("rankedScore", std::to_string(nsLL  (st, "ranked_score")),  init);
            Helpers::setField("totalScore",  std::to_string(nsLL  (st, "total_score")),   init);
            Helpers::setField("ppRank",      std::to_string(nsInt  (st, "global_rank")),  init);
            Helpers::setField("countryRank", std::to_string(nsInt  (st, "country_rank")), init);
            Helpers::setField("pp",          std::to_string(nsFloat(st, "pp")),           init);
            Helpers::setField("acc",         std::to_string(nsFloat(st, "hit_accuracy")), init);
            Helpers::setField("playtime",    std::to_string(nsLL  (st, "play_time")),     init);
            Helpers::setField("playcount",   std::to_string(nsInt  (st, "play_count")),   init);
            Helpers::setField("maxCombo",    std::to_string(nsInt  (st, "maximum_combo")),init);
            Helpers::setField("replayViews", std::to_string(nsInt  (st, "replays_watched_by_others")),init);
            Helpers::setField("totalHits",   std::to_string(nsLL  (st, "total_hits")),    init);
            Helpers::setField("silverSS",    std::to_string(nsInt  (gc, "ssh")),          init);
            Helpers::setField("goldSS",      std::to_string(nsInt  (gc, "ss")),           init);
            Helpers::setField("silverS",     std::to_string(nsInt  (gc, "sh")),           init);
            Helpers::setField("goldS",       std::to_string(nsInt  (gc, "s")),            init);
            Helpers::setField("a",           std::to_string(nsInt  (gc, "a")),            init);

            Helpers::setField("totalSS", std::to_string(Helpers::getIntField("silverSS") + Helpers::getIntField("goldSS")), init);
            Helpers::setField("totalS",  std::to_string(Helpers::getIntField("silverS")  + Helpers::getIntField("goldS")),  init);
            Helpers::setField("clears",  std::to_string(Helpers::getIntField("totalSS")  + Helpers:: getIntField("totalS") + Helpers::getIntField("a")), init);

            Helpers::calcDifference();
            return r.status_code;
        }
        catch (const nlohmann::json::exception& e) { Core::Console::writeLog("osu_api() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
        catch (const cpr::Error& e)                { Core::Console::writeLog("osu_api() -> CPR: "  + e.message,             true, 255, 0, 0); }
        catch (const std::exception& e)            { Core::Console::writeLog("osu_api() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
        catch (...)                                { Core::Console::writeLog("osu_api() -> Unknown",                        true, 255, 0, 0); }
        return -1;
    }

    namespace extended {

        static void respektive_api(bool init) {
            try {
                const std::string modeParam = std::to_string(static_cast<int>(Core::Config::application.gameMode));

                cpr::Response r_user = cpr::Get(
                    cpr::Url{ "https://score.respektive.pw/u/" + std::to_string(Core::Config::application.activeId()) + "?m=" + modeParam },
                    cpr::Header{ { "Content-Type", "application/json" } },
                    cpr::Timeout{ Status::TIMEOUT_MS }
                );
                Core::Console::writeLog("respektive_api() -> Status: " + std::to_string(r_user.status_code), false, 0, 255, 0);
                Status::statusRespektive = { r_user.status_code, true, std::chrono::system_clock::now() };

                nlohmann::json _user = nlohmann::json::parse(r_user.text);
                if (!_user.is_array() || _user.empty()) return;

                Helpers::setField("scoreRank", std::to_string(_user[0]["rank"].get<int>()), init);
                Helpers::calcDifference();

                cpr::Response r_target = cpr::Get(
                    cpr::Url{ "https://score.respektive.pw/rank/" + std::to_string(_user[0]["rank"].get<int>() - 1) + "?m=" + modeParam },
                    cpr::Header{ { "Content-Type", "application/json" } },
                    cpr::Timeout{ Status::TIMEOUT_MS }
                );
                Core::Console::writeLog("respektive_api() -> Status: " + std::to_string(r_target.status_code), false, 0, 255, 0);

                nlohmann::json _target = nlohmann::json::parse(r_target.text);
                if (!_target.is_array() || _target.empty()) return;

                Helpers::setField("targetRank",  std::to_string(_target[0]["rank"].get<int>()));
                Helpers::setField("targetUser",  _target[0]["username"].get<std::string>());
                Helpers::setField("targetScore", std::to_string(_target[0]["score"].get<long long>() - _user[0]["score"].get<long long>()));

                auto& arr = Core::Config::data::arr;
                if (arr[Core::Config::data::getIndex("targetRank")].current  == "Invalid") arr[Core::Config::data::getIndex("targetRank")].current  = "";
                if (arr[Core::Config::data::getIndex("targetScore")].current == "Invalid") arr[Core::Config::data::getIndex("targetScore")].current = "";
            }
            catch (const nlohmann::json::exception& e) { Core::Console::writeLog("respektive_api() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
            catch (const cpr::Error& e)                { Core::Console::writeLog("respektive_api() -> CPR: "  + e.message,             true, 255, 0, 0); }
            catch (const std::exception& e)            { Core::Console::writeLog("respektive_api() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
            catch (...)                                { Core::Console::writeLog("respektive_api() -> Unknown",                        true, 255, 0, 0); }
        }

        static void inspector_api(bool init) {
            try {
                cpr::Response r = cpr::Get(
                    cpr::Url{ "https://api.kirino.sh/inspector/users/stats/" + std::to_string(Core::Config::application.activeId()) + "?skipDailyData=true&skipOsuData=true&skipExtras=true" },
                    cpr::Header{ { "Content-Type", "application/json" } },
                    cpr::Timeout{ Status::TIMEOUT_MS }
                );
                Core::Console::writeLog("inspector_api() -> Status: " + std::to_string(r.status_code), false, 0, 255, 0);
                Status::statusInspector = { r.status_code, true, std::chrono::system_clock::now() };
                if (r.status_code != 200) return;

                nlohmann::json _j = nlohmann::json::parse(r.text);
                if (!_j.is_object() || !_j.contains("stats") || !_j["stats"].is_object()) return;

                const nlohmann::json& stats = _j["stats"];
                Helpers::setField("b",           std::to_string(stats.value("b", 0)),        init);
                Helpers::setField("c",           std::to_string(stats.value("c", 0)),        init);
                Helpers::setField("d",           std::to_string(stats.value("d", 0)),        init);
                Helpers::setField("totalClears", stats.value("clears",     std::string("")), init);
                Helpers::setField("completion",  stats.value("completion", std::string("")), init);
                Helpers::calcDifference();
            }
            catch (const nlohmann::json::exception& e) { Core::Console::writeLog("inspector_api() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
            catch (const cpr::Error& e)                { Core::Console::writeLog("inspector_api() -> CPR: "  + e.message,             true, 255, 0, 0); }
            catch (const std::exception& e)            { Core::Console::writeLog("inspector_api() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
            catch (...)                                { Core::Console::writeLog("inspector_api() -> Unknown",                        true, 255, 0, 0); }
        }

    }

}