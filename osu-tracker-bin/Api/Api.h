#pragma once
#include <thread>
#include <Api/Status.h>
#include <Api/Helpers.h>
#include <Api/Bancho.h>
#include <Api/PrivateServer/Titanic.h>
#include <Api/Update.h>
#include <Api/Simulate.h>

namespace Api {

    static void fetch_api_data(bool init) {
        if (Status::simulateMode) {
            Simulate::simulate(init);
            return;
        }
        switch (Core::Config::application.server) {
            case Core::Config::server::bancho: {
                std::thread t1([init]() {
                    if (Bancho::api_auth() != 200) {
                        Status::init_api_failed = true;
                        return;
                    }
                    if (Status::init_api_failed && !init)
                        Status::init_api_failed = false;
                    Bancho::api(init);
                });
                std::thread t2(Bancho::extended::respektive_api, init);
                std::thread t3(Bancho::extended::inspector_api, init);
                t1.join(); t2.join(); t3.join();

                if (Status::init_api_failed) return;
                break;
            }
            case Core::Config::server::titanic: {
                if (PrivateServer::Titanic::titanic(init) != 200) {
                    if (init) Status::init_api_failed = true;
                    return;
                }
                if (Status::init_api_failed && !init) {
                    Status::init_api_failed = false;
                    PrivateServer::Titanic::titanic(true);
                }
                break;
            }
        }
    }

}