#pragma once
#include <crow.h>
#include <filesystem>
#include <Web/WebState.h>
#include <Web/RouteRegister.h>

class CustomLogger : public crow::ILogHandler {
public:
	CustomLogger() {}
	void log(std::string message, crow::LogLevel level) {
		auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm* timeInfo = localtime(&currentTime);
		char dateBuffer[20];
		strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", timeInfo);
		char timeBuffer[9];
		strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", timeInfo);
		switch (level) {
			#ifdef DEBUG_BUILD
				case crow::LogLevel::Debug:
					Core::Console::setColorRGB_f(100, 100, 100);
					std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
					Core::Console::resetColor();
					std::cout << "[";
					Core::Console::setColor(Core::Console::conCol::b_defaultColor, Core::Console::conCol::f_white);
					std::cout << "Debug";
					Core::Console::resetColor();
					std::cout << "] ";
					Core::Console::setColorRGB_f(255, 255, 255);
					std::cout << message << "\n";
					Core::Console::resetColor();
					break;
				case crow::LogLevel::Info:
					Core::Console::setColorRGB_f(100, 100, 100);
					std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
					Core::Console::resetColor();
					std::cout << "[";
					Core::Console::setColor(Core::Console::conCol::b_defaultColor, Core::Console::conCol::f_cyan);
					std::cout << "Info";
					Core::Console::resetColor();
					std::cout << "] ";
					Core::Console::setColorRGB_f(255, 255, 255);
					std::cout << message << "\n";
					Core::Console::resetColor();
					break;
			#endif
			case crow::LogLevel::Warning:
				Core::Console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				Core::Console::resetColor();
				std::cout << "[";
				Core::Console::setColor(Core::Console::conCol::b_defaultColor, Core::Console::conCol::f_yellow);
				std::cout << "Warning";
				Core::Console::resetColor();
				std::cout << "] ";
				Core::Console::setColorRGB_f(255, 255, 255);
				std::cout << message << "\n";
				Core::Console::resetColor();
				break;
			case crow::LogLevel::Error:
				Core::Console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				Core::Console::resetColor();
				std::cout << "[";
				Core::Console::setColor(Core::Console::conCol::b_defaultColor, Core::Console::conCol::f_red);
				std::cout << "Error";
				Core::Console::resetColor();
				std::cout << "] ";
				Core::Console::setColorRGB_f(255, 255, 255);
				std::cout << message << "\n";
				Core::Console::resetColor();
				break;
			case crow::LogLevel::Critical:
				Core::Console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				Core::Console::resetColor();
				Core::Console::setColor(Core::Console::conCol::b_red, Core::Console::conCol::f_white);
				std::cout << "[Critical]";
				Core::Console::resetColor();
				Core::Console::setColorRGB_f(255, 255, 255);
				std::cout << " " << message << "\n";
				Core::Console::resetColor();
				break;
		}
	}
};

namespace Web::WebHandler {

	static bool start(bool skipInit = false) {
		CustomLogger logger;
		crow::logger::setHandler(&logger);

		if (!skipInit) {
			crow::mustache::set_base("./www/");
			crow::mustache::set_global_base("./www/");

			Web::RouteRegister::registerAll(app);
			app.add_static_dir();
		}

		Core::Console::writeLog("Starting Web Server...", true);
		Core::Console::writeLog("Web Server should be accessible under:", true);
		Core::Console::writeLog("#####################", true, 255, 255, 0);
		std::string url = "http://" + (std::string)OSU_TRACKER_WEBSERVER_HOST + ":" + std::to_string(OSU_TRACKER_WEBSERVER_PORT);
		Core::Console::writeLog(url, true, 0, 140, 255);
		Core::Console::writeLog("#####################", true, 255, 255, 0);

		app.bindaddr(OSU_TRACKER_WEBSERVER_HOST).port(OSU_TRACKER_WEBSERVER_PORT).signal_clear().run(); // blocking

		Core::Console::writeLog("Web Server Terminated", true, 255, 0, 0);
		return shutdown_webServer;
	}
}