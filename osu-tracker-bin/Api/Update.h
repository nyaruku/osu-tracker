#pragma once
#include <fstream>
#include <json.hpp>
#include <cpr/cpr.h>
#include <Api/Status.h>
#include <Core/Console.h>
#include <Core/Helpers.h>

namespace Api::Update {

    static bool download(const nlohmann::json& releaseJson) {
        const std::string target = "x86-release-" + std::string(OSU_TRACKER_PLATFORM) + ".zip";
        for (const auto& asset : releaseJson["assets"]) {
            if (std::string(asset["name"]) != target) continue;

            Core::Console::writeLog("Downloading " + target + "...", true, 173, 216, 230);
            cpr::Response zip = cpr::Get(
                cpr::Url{ asset["browser_download_url"].get<std::string>() },
                cpr::Timeout{ Status::TIMEOUT_MS }
            );
            if (zip.status_code != 200) {
                Core::Console::writeLog("Download failed. Status: " + std::to_string(zip.status_code), true, 255, 0, 0);
                return false;
            }
            std::ofstream out("update.zip", std::ios::binary);
            if (!out) {
                Core::Console::writeLog("Failed to open update.zip for writing.", true, 255, 0, 0);
                return false;
            }
            out.write(zip.text.c_str(), zip.text.size());
            out.close();
            Core::Console::writeLog(target + " downloaded > update.zip.", true, 0, 255, 0);
            return true;
        }
        Core::Console::writeLog("Invalid target: " + target, true, 255, 0, 0);
        return false;
    }

    static bool update() {
        try {
            cpr::Response r = cpr::Get(
                cpr::Url{ "https://api.github.com/repos/nyaruku/osu-tracker/releases/latest" },
                cpr::Timeout{ Status::TIMEOUT_MS }
            );
            if (r.status_code != 200) {
                Core::Console::writeLog("Failed to get latest release.", true, 255, 0, 0);
                return false;
            }
            nlohmann::json request   = nlohmann::json::parse(r.text);
            std::string    signedVer = Core::Helpers::replace(Core::Helpers::replace(std::string(request["tag_name"]), "v", ""), ".", "");
            if (!signedVer.empty() && signedVer[0] == '0') signedVer = signedVer.substr(1);

            Core::Console::writeLog("Signed Version: " + signedVer + " (" + std::string(request["tag_name"]) + ")", true, 111, 163, 247);
            const int signedVersion = std::stoi(signedVer);

            #if OSU_TRACKER_UPDATE_EQUAL == 1
                if (signedVersion == std::stoi(OSU_TRACKER_VERSION_SIGNED)) return download(request);
            #endif
            if (signedVersion > std::stoi(OSU_TRACKER_VERSION_SIGNED)) return download(request);
            Core::Console::writeLog("No updates found.", true, 0, 255, 0);
            return false;
        }
        catch (const nlohmann::json::exception& e) { Core::Console::writeLog("update() -> JSON: " + std::string(e.what()), true, 255, 0, 0); }
        catch (const cpr::Error& e)                { Core::Console::writeLog("update() -> CPR: "  + e.message,             true, 255, 0, 0); }
        catch (const std::exception& e)            { Core::Console::writeLog("update() -> Std: "  + std::string(e.what()), true, 255, 0, 0); }
        catch (...)                                { Core::Console::writeLog("update() -> Unknown",                        true, 255, 0, 0); }
        return false;
    }

}