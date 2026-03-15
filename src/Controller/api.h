#pragma once
#include <cmath>
#include <thread>
#include <array>
#include <chrono>
#include <Controller/Core/config.h>
#include <Controller/Core/external.h>

namespace api {

	static constexpr long TIMEOUT_MS = OSU_TRACKER_API_TIMEOUT;

	inline bool init_api_failed = false;
	inline bool simulateMode    = false;

	struct EndpointStatus {
		int  code       = 0;
		bool everCalled = false;
		std::chrono::system_clock::time_point lastCall;
	};

	inline EndpointStatus statusAuth;
	inline EndpointStatus statusBancho;
	inline EndpointStatus statusRespektive;
	inline EndpointStatus statusInspector;
	inline EndpointStatus statusTitanic;

	// -- Helpers ------------------------------------------------------------------

	// Sets a field's current value; also mirrors it to init when requested.
	static void setField(const std::string& key, const std::string& value, bool setInit = false) {
		int idx = config::data::getIndex(key);
		config::data::arr[idx].current = value;
		if (setInit) config::data::arr[idx].init = value;
	}

	static int getIntField(const std::string& key) {
		return std::stoi(config::data::arr[config::data::getIndex(key)].current);
	}

	static void calcDifference() {
		for (const config::dataEntry& data : config::data::arr) {
			if (data.init.empty() || data.current.empty()) continue;
			int idx = config::data::getIndex(data.key);
			switch (data.dataType) {
				case config::dataType::d_int:
					config::data::arr[idx].change = std::to_string(std::stoi(data.current) - std::stoi(data.init));
					break;
				case config::dataType::d_float:
					config::data::arr[idx].change = std::to_string(std::stof(data.current) - std::stof(data.init));
					break;
				case config::dataType::d_longLong:
					config::data::arr[idx].change = std::to_string(std::stoll(data.current) - std::stoll(data.init));
					break;
			}
		}
	}

	// -- Private Servers ----------------------------------------------------------

	namespace pServer {

		static int titanic(bool init) {
			console::writeLog(std::string("titanic_api(init) -> ") + ext::bool2str(init), false, 0, 0, 255);
			try {
				cpr::Response r_users, r_stats;

				std::thread t1([&]() {
					r_users = cpr::Get(
						cpr::Url{ "https://api.titanic.sh/users/" + std::to_string(config::application.activeId()) },
						cpr::Header{ { "Content-Type", "application/json" }, { "Accept", "application/json" } },
						cpr::Timeout{ TIMEOUT_MS }
					);
					console::writeLog("titanic_api(/users) -> Status: " + std::to_string(r_users.status_code), false, 0, 255, 0);
				});
				std::thread t2([&]() {
					r_stats = cpr::Get(
						cpr::Url{ "https://api.titanic.sh/stats" },
						cpr::Header{ { "Content-Type", "application/json" }, { "Accept", "application/json" } },
						cpr::Timeout{ TIMEOUT_MS }
					);
					console::writeLog("titanic_api(/stats) -> Status: " + std::to_string(r_stats.status_code), false, 0, 255, 0);
				});
				t1.join(); t2.join();

				statusTitanic = { r_users.status_code, true, std::chrono::system_clock::now() };

				nlohmann::json _j  = nlohmann::json::parse(r_users.text);
				nlohmann::json _j2 = nlohmann::json::parse(r_stats.text);

				const int         modeIdx   = static_cast<int>(config::application.gameMode);
				const std::string modeStr   = std::to_string(modeIdx);
				const int totalMaps =
					_j2["beatmap_modes"][modeStr]["count_ranked"].get<int>()    +
					_j2["beatmap_modes"][modeStr]["count_approved"].get<int>()  +
					_j2["beatmap_modes"][modeStr]["count_qualified"].get<int>() +
					_j2["beatmap_modes"][modeStr]["count_loved"].get<int>();
				// For non-std modes, std maps are playable as converts so add them to the denominator.
				const int completionDenom = (modeIdx == 0) ? totalMaps : totalMaps +
					_j2["beatmap_modes"]["0"]["count_ranked"].get<int>()    +
					_j2["beatmap_modes"]["0"]["count_approved"].get<int>()  +
					_j2["beatmap_modes"]["0"]["count_qualified"].get<int>() +
					_j2["beatmap_modes"]["0"]["count_loved"].get<int>();

				config::user.username = _j["name"].get<std::string>();

				// avatar
				{
					std::string url = _j.value("avatar_url",
						"https://a.titanic.sh/" + std::to_string(config::application.activeId()));
					if (url != config::user.avatarUrl) {
						config::user.avatarUrl = url;
						cpr::Response av = cpr::Get(cpr::Url{ url }, cpr::Timeout{ TIMEOUT_MS });
						if (av.status_code == 200) {
							std::lock_guard<std::mutex> lk(config::user.avatarMutex);
							config::user.avatarBytes.assign(av.text.begin(), av.text.end());
							config::user.avatarDirty = true;
						}
					}
				}
				const auto& s = _j["stats"][modeIdx];
				setField("level",       std::to_string(ext::getLevelFromScore(s["tscore"].get<long long>())), init);
				setField("rankedScore", std::to_string(s["rscore"].get<long long>()),    init);
				setField("totalScore",  std::to_string(s["tscore"].get<long long>()),    init);
				setField("ppRank",      std::to_string(s["rank"].get<int>()),            init);
				setField("pp",          std::to_string(s["pp"].get<float>()),            init);
				setField("ppv1",        std::to_string(s["ppv1"].get<float>()),          init);
				setField("acc",         std::to_string(s["acc"].get<float>() * 100),     init);
				setField("playtime",    std::to_string(s["playtime"].get<long long>()),  init);
				setField("playcount",   std::to_string(s["playcount"].get<int>()),       init);
				setField("totalHits",   std::to_string(s["total_hits"].get<long long>()),   init);
				setField("silverSS",    std::to_string(s["xh_count"].get<int>()),        init);
				setField("goldSS",      std::to_string(s["x_count"].get<int>()),         init);
				setField("silverS",     std::to_string(s["sh_count"].get<int>()),        init);
				setField("goldS",       std::to_string(s["s_count"].get<int>()),         init);
				setField("a",           std::to_string(s["a_count"].get<int>()),         init);
				setField("b",           std::to_string(s["b_count"].get<int>()),         init);
				setField("c",           std::to_string(s["c_count"].get<int>()),         init);
				setField("d",           std::to_string(s["d_count"].get<int>()),         init);
				setField("maxCombo",    std::to_string(s["max_combo"].get<int>()),       init);
				setField("replayViews", std::to_string(s["replay_views"].get<int>()),    init);

				// Rankings (already in the /users response)
				const auto& rnk = _j["rankings"][modeStr];
				setField("countryRank", std::to_string(rnk["performance"]["country"].get<int>()), init);
				setField("ppv1Rank",    std::to_string(rnk["ppv1"]["global"].get<int>()),         init);
				setField("scoreRank",   std::to_string(rnk["rscore"]["global"].get<int>()),       init);

				// Computed grade aggregates
				setField("totalSS",     std::to_string(getIntField("silverSS") + getIntField("goldSS")), init);
				setField("totalS",      std::to_string(getIntField("silverS")  + getIntField("goldS")),  init);
				setField("clears",      std::to_string(getIntField("totalSS")  + getIntField("totalS") + getIntField("a")), init);
				setField("totalClears", std::to_string(getIntField("clears") + getIntField("b") + getIntField("c") + getIntField("d")), init);
				setField("completion",  std::to_string((static_cast<float>(getIntField("totalClears")) / static_cast<float>(completionDenom)) * 100), init);

				calcDifference();
				return r_users.status_code;
			}
			catch (const nlohmann::json::exception& e) { console::writeLog("titanic_api() -> JSON: "    + std::string(e.what()), true, 255, 0, 0); }
			catch (const cpr::Error& e)                { console::writeLog("titanic_api() -> CPR: "     + e.message,             true, 255, 0, 0); }
			catch (const std::exception& e)            { console::writeLog("titanic_api() -> Std: "     + std::string(e.what()), true, 255, 0, 0); }
			catch (...)                                { console::writeLog("titanic_api() -> Unknown",                           true, 255, 0, 0); }
			return -1;
		}

	} // namespace pServer

	// -- osu! Bancho --------------------------------------------------------------

	namespace osu {

		inline std::string auth_token;

		static int api_auth() {
			try {
				const std::string body =
					R"({"client_id":)"       + std::to_string(config::application.clientId) +
					R"(, "client_secret":")" + config::application.clientSecret +
					R"(", "grant_type":"client_credentials", "scope":"public"})";

				cpr::Response r = cpr::Post(
					cpr::Url{ "https://osu.ppy.sh/oauth/token" },
					cpr::Body{ body },
					cpr::Header{ { "Content-Type", "application/json" } },
					cpr::Timeout{ TIMEOUT_MS }
				);

				nlohmann::json response = nlohmann::json::parse(r.text);
				auth_token = "Bearer " + response.value("access_token", "");
				console::writeLog("api_auth() -> Status: " + std::to_string(r.status_code), false, 0, 255, 0);
				statusAuth = { r.status_code, true, std::chrono::system_clock::now() };
				return r.status_code;
			}
			catch (const nlohmann::json::exception& e) { console::writeLog("api_auth() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
			catch (const cpr::Error& e)                { console::writeLog("api_auth() -> CPR: "  + e.message,             true, 255, 0, 0); }
			catch (const std::exception& e)            { console::writeLog("api_auth() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
			catch (...)                                { console::writeLog("api_auth() -> Unknown",                        true, 255, 0, 0); }
			return -1;
		}

		static int api(bool init) {
			try {
				static constexpr std::array<const char*, 4> modeNames = { "osu", "taiko", "fruits", "mania" };
				const std::string mode = modeNames[static_cast<int>(config::application.gameMode)];

				cpr::Response r = cpr::Get(
					cpr::Url{ "https://osu.ppy.sh/api/v2/users/" + std::to_string(config::application.activeId()) + "/" + mode + "?key=id" },
					cpr::Header{
						{ "Content-Type",  "application/json" },
						{ "Accept",        "application/json" },
						{ "Authorization", auth_token }
					},
					cpr::Timeout{ TIMEOUT_MS }
				);
				console::writeLog("api() -> Status: " + std::to_string(r.status_code), false, 0, 255, 0);
				statusBancho = { r.status_code, true, std::chrono::system_clock::now() };
				if (r.status_code != 200) return r.status_code;

				nlohmann::json _j = nlohmann::json::parse(r.text);
				if (!_j.contains("statistics") || _j["statistics"].is_null()) return r.status_code;
				const auto& st = _j.at("statistics");
				const auto& gc = st.at("grade_counts");
				config::user.username = _j["username"].get<std::string>();

				// avatar
				{
					std::string url = _j.value("avatar_url", "https://a.ppy.sh/" + std::to_string(config::application.activeId()));
					if (url != config::user.avatarUrl) {
						config::user.avatarUrl = url;
						cpr::Response av = cpr::Get(cpr::Url{ url }, cpr::Timeout{ TIMEOUT_MS });
						if (av.status_code == 200) {
							std::lock_guard<std::mutex> lk(config::user.avatarMutex);
							config::user.avatarBytes.assign(av.text.begin(), av.text.end());
							config::user.avatarDirty = true;
						}
					}
				}

				// Helpers: Bancho API returns null (not 0) for rank/pp/etc. when a user
				// has no plays in a given mode. These return a safe default instead.
				auto nsInt   = [&](const nlohmann::json& j, const char* k, int       d = 0   ) {
					const auto it = j.find(k);
					return (it != j.end() && !it->is_null()) ? it->get<int>()       : d;
				};
				auto nsFloat = [&](const nlohmann::json& j, const char* k, float     d = 0.f ) {
					const auto it = j.find(k);
					return (it != j.end() && !it->is_null()) ? it->get<float>()     : d;
				};
				auto nsLL    = [&](const nlohmann::json& j, const char* k, long long d = 0LL ) {
					const auto it = j.find(k);
					return (it != j.end() && !it->is_null()) ? it->get<long long>() : d;
				};

				setField("level",       std::to_string(ext::getLevelFromScore(nsLL  (st, "total_score"))),  init);
				setField("rankedScore", std::to_string(nsLL  (st, "ranked_score")),  init);
				setField("totalScore",  std::to_string(nsLL  (st, "total_score")),   init);
				setField("ppRank",      std::to_string(nsInt  (st, "global_rank")),  init);
				setField("pp",          std::to_string(nsFloat(st, "pp")),           init);
				setField("acc",         std::to_string(nsFloat(st, "hit_accuracy")), init);
				setField("playtime",    std::to_string(nsLL  (st, "play_time")),     init);
				setField("playcount",   std::to_string(nsInt  (st, "play_count")),   init);
				setField("totalHits",   std::to_string(nsLL  (st, "total_hits")),    init);
				setField("silverSS",    std::to_string(nsInt  (gc, "ssh")),          init);
				setField("goldSS",      std::to_string(nsInt  (gc, "ss")),           init);
				setField("silverS",     std::to_string(nsInt  (gc, "sh")),           init);
				setField("goldS",       std::to_string(nsInt  (gc, "s")),            init);
				setField("a",           std::to_string(nsInt  (gc, "a")),            init);

				// Computed grade aggregates
				setField("totalSS", std::to_string(getIntField("silverSS") + getIntField("goldSS")), init);
				setField("totalS",  std::to_string(getIntField("silverS")  + getIntField("goldS")),  init);
				setField("clears",  std::to_string(getIntField("totalSS")  + getIntField("totalS") + getIntField("a")), init);

				calcDifference();
				return r.status_code;
			}
			catch (const nlohmann::json::exception& e) { console::writeLog("osu_api() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
			catch (const cpr::Error& e)                { console::writeLog("osu_api() -> CPR: "  + e.message,             true, 255, 0, 0); }
			catch (const std::exception& e)            { console::writeLog("osu_api() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
			catch (...)                                { console::writeLog("osu_api() -> Unknown",                        true, 255, 0, 0); }
			return -1;
		}

		namespace extended {

			static void respektive_api(bool init) {
				try {
					const std::string modeParam = std::to_string(static_cast<int>(config::application.gameMode));

					cpr::Response r_user = cpr::Get(
						cpr::Url{ "https://score.respektive.pw/u/" + std::to_string(config::application.activeId()) + "?m=" + modeParam },
						cpr::Header{ { "Content-Type", "application/json" } },
                                        cpr::Timeout{ TIMEOUT_MS }
					);
					console::writeLog("respektive_api() -> Status: " + std::to_string(r_user.status_code), false, 0, 255, 0);
					statusRespektive = { r_user.status_code, true, std::chrono::system_clock::now() };

					nlohmann::json _user = nlohmann::json::parse(r_user.text);
					if (!_user.is_array() || _user.empty()) return;

					setField("scoreRank", std::to_string(_user[0]["rank"].get<int>()), init);
					calcDifference();

					cpr::Response r_target = cpr::Get(
						cpr::Url{ "https://score.respektive.pw/rank/" + std::to_string(_user[0]["rank"].get<int>() - 1) + "?m=" + modeParam },
						cpr::Header{ { "Content-Type", "application/json" } },
                                        cpr::Timeout{ TIMEOUT_MS }
					);
					console::writeLog("respektive_api() -> Status: " + std::to_string(r_target.status_code), false, 0, 255, 0);

					nlohmann::json _target = nlohmann::json::parse(r_target.text);
					if (!_target.is_array() || _target.empty()) return;

					setField("targetRank",  std::to_string(_target[0]["rank"].get<int>()));
					setField("targetUser",  _target[0]["username"].get<std::string>());
					setField("targetScore", std::to_string(_target[0]["score"].get<long long>() - _user[0]["score"].get<long long>()));

					// Clear invalid sentinel values
					auto& arr = config::data::arr;
					if (arr[config::data::getIndex("targetRank")].current  == "Invalid") arr[config::data::getIndex("targetRank")].current  = "";
					if (arr[config::data::getIndex("targetScore")].current == "Invalid") arr[config::data::getIndex("targetScore")].current = "";
				}
				catch (const nlohmann::json::exception& e) { console::writeLog("respektive_api() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
				catch (const cpr::Error& e)                { console::writeLog("respektive_api() -> CPR: "  + e.message,             true, 255, 0, 0); }
				catch (const std::exception& e)            { console::writeLog("respektive_api() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
				catch (...)                                { console::writeLog("respektive_api() -> Unknown",                        true, 255, 0, 0); }
			}

			static void inspector_api(bool init) {
				try {
					cpr::Response r = cpr::Get(
						cpr::Url{ "https://api.kirino.sh/inspector/users/stats/" + std::to_string(config::application.activeId()) + "?skipDailyData=true&skipOsuData=true&skipExtras=true" },
						cpr::Header{ { "Content-Type", "application/json" } },
                                        cpr::Timeout{ TIMEOUT_MS }
					);
					console::writeLog("inspector_api() -> Status: " + std::to_string(r.status_code), false, 0, 255, 0);
					statusInspector = { r.status_code, true, std::chrono::system_clock::now() };
					if (r.status_code != 200) return;

					nlohmann::json _j = nlohmann::json::parse(r.text);
					if (!_j.is_object() || !_j.contains("stats") || !_j["stats"].is_object()) return;

					const nlohmann::json& stats = _j["stats"];
					setField("b",           std::to_string(stats.value("b", 0)),                init);
					setField("c",           std::to_string(stats.value("c", 0)),                init);
					setField("d",           std::to_string(stats.value("d", 0)),                init);
					setField("totalClears", stats.value("clears",     std::string("")),         init); // MIO >:(
					setField("completion",  stats.value("completion", std::string("")),         init); // MIO >:(
					calcDifference();
				}
				catch (const nlohmann::json::exception& e) { console::writeLog("inspector_api() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
				catch (const cpr::Error& e)                { console::writeLog("inspector_api() -> CPR: "  + e.message,             true, 255, 0, 0); }
				catch (const std::exception& e)            { console::writeLog("inspector_api() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
				catch (...)                                { console::writeLog("inspector_api() -> Unknown",                        true, 255, 0, 0); }
			}

		} // namespace extended

	}

	static bool download(const nlohmann::json& releaseJson) {
		const std::string target = "x86-release-" + std::string(OSU_TRACKER_PLATFORM) + ".zip";
		for (const auto& asset : releaseJson["assets"]) {
			if (std::string(asset["name"]) != target) continue;

			console::writeLog("Downloading " + target + "...", true, 173, 216, 230);
			cpr::Response zip = cpr::Get(
				cpr::Url{ asset["browser_download_url"].get<std::string>() },
				cpr::Timeout{ TIMEOUT_MS }
			);
			if (zip.status_code != 200) {
				console::writeLog("Download failed. Status: " + std::to_string(zip.status_code), true, 255, 0, 0);
				return false;
			}
			std::ofstream out("update.zip", std::ios::binary);
			if (!out) {
				console::writeLog("Failed to open update.zip for writing.", true, 255, 0, 0);
				return false;
			}
			out.write(zip.text.c_str(), zip.text.size());
			out.close();
			console::writeLog(target + " downloaded > update.zip.", true, 0, 255, 0);
			return true;
		}
		console::writeLog("Invalid target: " + target, true, 255, 0, 0);
		return false;
	}

	static bool update() {
		try {
			cpr::Response r = cpr::Get(
				cpr::Url{ "https://api.github.com/repos/nyaruku/osu-tracker/releases/latest" },
				cpr::Timeout{ TIMEOUT_MS }
			);
			if (r.status_code != 200) {
				console::writeLog("Failed to get latest release.", true, 255, 0, 0);
				return false;
			}
			nlohmann::json request    = nlohmann::json::parse(r.text);
			std::string    signedVer  = ext::replace(ext::replace(std::string(request["tag_name"]), "v", ""), ".", "");
			if (!signedVer.empty() && signedVer[0] == '0') signedVer = signedVer.substr(1);

			console::writeLog("Signed Version: " + signedVer + " (" + std::string(request["tag_name"]) + ")", true, 111, 163, 247);
			const int signedVersion = std::stoi(signedVer);

			#if OSU_TRACKER_UPDATE_EQUAL == 1
				if (signedVersion == std::stoi(OSU_TRACKER_VERSION_SIGNED)) return download(request);
			#endif
			if (signedVersion > std::stoi(OSU_TRACKER_VERSION_SIGNED)) return download(request);
			return false;
		}
		catch (const nlohmann::json::exception& e) { console::writeLog("update() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
		catch (const cpr::Error& e)                { console::writeLog("update() -> CPR: "  + e.message,             true, 255, 0, 0); }
		catch (const std::exception& e)            { console::writeLog("update() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
		catch (...)                                { console::writeLog("update() -> Unknown",                        true, 255, 0, 0); }
		return false;
	}

	static void simulate(bool init) {
		if (!init) return;
		config::user.username = "Player";
		setField("level",       "100",          true);
		setField("scoreRank",   "100",          true);
		setField("ppRank",      "50000",        true);
		setField("ppv1Rank",    "50000",        true);
		setField("countryRank", "2000",         true);
		setField("pp",          "1000",         true);
		setField("ppv1",        "1000",         true);
		setField("acc",         "99",           true);
		setField("playtime",    "100000",       true);
		setField("playcount",   "25000",        true);
		setField("maxCombo",    "5000",         true);
		setField("totalHits",   "5000000",      true);
		setField("replayViews", "1000",         true);
		setField("rankedScore", "10000000000",  true);
		setField("totalScore",  "10000000000",  true);
		setField("clears",      "10000",        true);
		setField("totalClears", "15000",        true);
		setField("silverSS",    "1000",         true);
		setField("goldSS",      "1000",         true);
		setField("silverS",     "1000",         true);
		setField("goldS",       "1000",         true);
		setField("a",           "1000",         true);
		setField("b",           "1000",         true);
		setField("c",           "1000",         true);
		setField("d",           "1000",         true);
		setField("totalSS",     "2000",         true);
		setField("totalS",      "2000",         true);
		setField("targetRank",  "99"   ,        true);
		setField("targetUser",  "OtherPlayer",  true);
		setField("targetScore", "20000000000",  true);
		setField("completion",  "50.00",        true);
		// static differences so the `UI shows non-zero change values
		setField("level",       "100.5");
		setField("scoreRank",   "50");
		setField("ppRank",      "20000");
		setField("ppv1Rank",    "20000");
		setField("countryRank", "500");
		setField("pp",          "2000");
		setField("ppv1",        "2000");
		setField("acc",         "99.5");
		setField("playtime",    "200000");
		setField("playcount",   "35000");
		setField("maxCombo",    "6000");
		setField("totalHits",   "6000000");
		setField("replayViews", "2000");
		setField("rankedScore", "20000000000");
		setField("totalScore",  "20000000000");
		setField("clears",      "20000");
		setField("totalClears", "20000");
		setField("silverSS",    "2000");
		setField("goldSS",      "2000");
		setField("silverS",     "2000");
		setField("goldS",       "2000");
		setField("a",           "2000");
		setField("b",           "2000");
		setField("c",           "2000");
		setField("d",           "2000");
		setField("totalSS",     "3000");
		setField("totalS",      "3000");
		setField("targetRank",  "99.5");
		setField("targetUser",  "OtherPlayer");
		setField("targetScore", "30000000000");
		setField("completion",  "51.00");
		calcDifference();
	}

	// -- Entry Point --------------------------------------------------------------

	static void fetch_api_data(bool init) {
		if (simulateMode) {
			simulate(init);
			return;
		}
		switch (config::application.server) {
			case config::server::bancho: {
				// t1 owns auth + main osu API (sequential within the thread).
				// t2 and t3 hit independent public endpoints and need no OAuth token,
				// so all three threads start at the same time.
				std::thread t1([init]() {
					if (osu::api_auth() != 200) {
						init_api_failed = true;
						return;
					}
					if (init_api_failed && !init)
						init_api_failed = false;
					osu::api(init);
				});
				std::thread t2(osu::extended::respektive_api, init);
				std::thread t3(osu::extended::inspector_api, init);
				t1.join(); t2.join(); t3.join();

				// If auth failed inside t1, reflect that on the caller's side.
				if (init_api_failed) return;
				break;
			}
			case config::server::titanic: {
				if (pServer::titanic(init) != 200) {
					if (init) init_api_failed = true;
					return;
				}
				if (init_api_failed && !init) {
					init_api_failed = false;
					pServer::titanic(true);
				}
				break;
			}
		}
	}

} // namespace api
