#pragma once
#include <algorithm>
#include <shared_mutex>
#include <string>
#include <Core/Config.h>

namespace Core::Formatter {

    inline std::string formatNumber(const std::string& numStr, bool showPlus = false, std::string c = "") {
        try {
            long long num = std::stoll(numStr);
            bool isNegative = num < 0;
            unsigned long long absNum = isNegative ? -num : num;
            std::string str = std::to_string(absNum);

            int insertPosition = static_cast<int>(str.length()) - 3;
            while (insertPosition > 0) {
                str.insert(insertPosition, ",");
                insertPosition -= 3;
            }
            if (c == "#")      str = "#" + str;
            if (isNegative)    str = "-" + str;
            else if (showPlus) str = "+" + str;
            return str;
        }
        catch (const std::exception&) { return "Invalid"; }
    }

    inline std::string formatFloat(const std::string& numStr, bool showPlus = false) {
        char* endPtr = nullptr;
        double num = std::strtod(numStr.c_str(), &endPtr);
        if (endPtr == numStr.c_str()) return "Invalid";

        char temp[64];
        snprintf(temp, sizeof(temp), "%.3f", num);
        std::string str(temp);

        bool isNegative = str[0] == '-';
        if (isNegative) str = str.substr(1);

        size_t dotPos       = str.find('.');
        std::string intPart = str.substr(0, dotPos);
        std::string decPart = str.substr(dotPos);

        int insertPosition = static_cast<int>(intPart.length()) - 3;
        while (insertPosition > 0) {
            intPart.insert(insertPosition, ",");
            insertPosition -= 3;
        }
        if (isNegative)    intPart = "-" + intPart;
        else if (showPlus) intPart = "+" + intPart;
        return intPart + decPart;
    }

    inline std::string formatPlaytime(const std::string& secondsStr, bool showPlus = false) {
        char* endPtr = nullptr;
        long seconds = std::strtol(secondsStr.c_str(), &endPtr, 10);
        if (endPtr == secondsStr.c_str()) return "Invalid";

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%dh %dm %ds",
            (int)(seconds / 3600), (int)((seconds % 3600) / 60), (int)(seconds % 60));
        std::string str(buffer);
        if (showPlus) str = "+" + str;
        return str;
    }

    inline void updateFormat() {
        Config::data::arrFormatted = Config::data::arr;

        std::sort(Config::data::arrFormatted.begin(), Config::data::arrFormatted.end(),
            [](const Config::dataEntry& a, const Config::dataEntry& b) { return a.sort < b.sort; });

        for (auto& data : Config::data::arrFormatted) {
            if (data.init.empty() && !data.single) {
                data.init = data.change = "";
                continue;
            }
            switch (Config::application.server) {
                case Config::server::bancho:  if (!data.banchoSupport)  continue; break;
                case Config::server::titanic: if (!data.titanicSupport) continue; break;
            }

            const bool noDiff    = !data.single && (data.init == data.current);
            const bool noCurrent = data.current.empty();
            if (noDiff) data.change = "";

            switch (data.dataType) {
                case Config::dataType::d_int:
                case Config::dataType::d_longLong: {
                    switch (data.formatType) {
                        case Config::formatType::f_int: {
                            data.init = formatNumber(data.init);
                            if (!noCurrent) data.current = formatNumber(data.current);
                            if (!noDiff) {
                                std::string s = formatNumber(data.change, true);
                                if (!s.empty() && s[0] == '-') data.positive = false;
                                data.change = s;
                            }
                            break;
                        }
                        case Config::formatType::f_rank: {
                            data.init = formatNumber(data.init, false, "#");
                            if (!noCurrent) data.current = formatNumber(data.current, false, "#");
                            if (!noDiff) {
                                std::string s = formatNumber(data.change, true);
                                if (!s.empty() && s[0] == '-') data.positive = false;
                                data.change = s;
                            }
                            break;
                        }
                        case Config::formatType::f_time: {
                            data.init = formatPlaytime(data.init);
                            if (!noCurrent) data.current = formatPlaytime(data.current);
                            if (!noDiff) data.change = formatPlaytime(data.change, true);
                            break;
                        }
                        default: break;
                    }
                    break;
                }
                case Config::dataType::d_float: {
                    switch (data.formatType) {
                        case Config::formatType::f_decimal: {
                            data.init = formatFloat(data.init);
                            if (!noCurrent) data.current = formatFloat(data.current);
                            if (!noDiff) {
                                std::string s = formatFloat(data.change, true);
                                if (!s.empty() && s[0] == '-') data.positive = false;
                                data.change = s;
                            }
                            break;
                        }
                        case Config::formatType::f_percent: {
                            data.init = formatFloat(data.init) + "%";
                            if (!noCurrent) data.current = formatFloat(data.current) + "%";
                            if (!noDiff) {
                                std::string s = formatFloat(data.change, true) + "%";
                                if (!s.empty() && s[0] == '-') data.positive = false;
                                data.change = s;
                            }
                            break;
                        }
                        default: break;
                    }
                    break;
                }
            }
        }
    }

}