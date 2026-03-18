#pragma once
#include <chrono>

namespace Api::Status {

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

}