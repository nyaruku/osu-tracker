#pragma once
#include <string>
#include <Core/Config.h>

namespace Api::Helpers {

    static void setField(const std::string& key, const std::string& value, bool setInit = false) {
        int idx = Core::Config::data::getIndex(key);
        Core::Config::data::arr[idx].current = value;
        if (setInit) Core::Config::data::arr[idx].init = value;
    }

    static int getIntField(const std::string& key) {
        return std::stoi(Core::Config::data::arr[Core::Config::data::getIndex(key)].current);
    }

    static void calcDifference() {
        for (const Core::Config::dataEntry& data : Core::Config::data::arr) {
            if (data.init.empty() || data.current.empty()) continue;
            int idx = Core::Config::data::getIndex(data.key);
            switch (data.dataType) {
                case Core::Config::dataType::d_int:
                    Core::Config::data::arr[idx].change = std::to_string(std::stoi(data.current) - std::stoi(data.init));
                    break;
                case Core::Config::dataType::d_float:
                    Core::Config::data::arr[idx].change = std::to_string(std::stof(data.current) - std::stof(data.init));
                    break;
                case Core::Config::dataType::d_longLong:
                    Core::Config::data::arr[idx].change = std::to_string(std::stoll(data.current) - std::stoll(data.init));
                    break;
            }
        }
    }

}