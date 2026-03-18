#pragma once
#include <shared_mutex>
#include <Core/Formatter.h>
#include <Api/Api.h>
#include <View/ui.h>

inline std::atomic<bool> fetch;
inline std::thread       fetchThread;

namespace Core::Session {

    inline std::atomic<bool>                     isOpen;
    inline std::atomic<int>                      fetchCount{ 0 };
    inline std::chrono::system_clock::time_point lastFetchCompleted;
    inline std::chrono::milliseconds             lastFetchDuration{ 0 };

    inline void fetchApiData(bool init) {
        while (fetch) {
            auto t0 = std::chrono::steady_clock::now();
            Api::fetch_api_data(init);
            init = false;
            {
                std::unique_lock<std::shared_mutex> lock(Config::data::dataMutex);
                Formatter::updateFormat();
                Config::writeStats();
            }
            lastFetchDuration  = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t0);
            lastFetchCompleted = std::chrono::system_clock::now();
            fetchCount++;
            std::this_thread::sleep_for(std::chrono::milliseconds(Config::application.apiInterval));
        }
    }

    inline void startFetchThread(bool init = false) {
        fetch = true;
        fetchThread = std::thread(fetchApiData, init);
    }

    inline int stopFetchThread() {
        fetch = false;
        if (fetchThread.joinable()) fetchThread.join();
        return 1;
    }

    inline void CloseWindow() {
        ui_mainTerminate();
        isOpen = false;
    }

    inline int OpenWindow() {
        isOpen = true;
        int result = ui_main();
        isOpen = false;
        return result;
    }

    inline void restartFetch() {
        static std::atomic<bool> restarting{ false };
        if (restarting.exchange(true)) return;
        std::thread([]() {
            fetch = false;
            if (fetchThread.joinable()) fetchThread.join();
            {
                std::unique_lock<std::shared_mutex> lk(Config::data::dataMutex);
                for (auto& e : Config::data::arr) { e.init = e.current = e.change = ""; }
                Config::data::arrFormatted.clear();
            }
            Config::user.avatarUrl = "";
            fetchCount = 0;
            fetch = true;
            fetchThread = std::thread(fetchApiData, true);
            restarting = false;
        }).detach();
    }

}