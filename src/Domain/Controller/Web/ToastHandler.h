#pragma once

namespace Web::ToastHandler {
    using json = nlohmann::json;

    inline json sendToast(std::string msg) {
        json _j;
        _j["cmd"] = "toast";
        _j["msg"] = msg;
        return _j;
    }
}
