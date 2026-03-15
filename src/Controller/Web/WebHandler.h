#pragma once
#include <crow.h>
#include <filesystem>
#include <Controller/Web/WebState.h>
#include <Controller/Web/RouteRegister.h>

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
					console::setColorRGB_f(100, 100, 100);
					std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
					console::resetColor();
					std::cout << "[";
					console::setColor(console::conCol::b_defaultColor, console::conCol::f_white);
					std::cout << "Debug";
					console::resetColor();
					std::cout << "] ";
					console::setColorRGB_f(255, 255, 255);
					std::cout << message << "\n";
					console::resetColor();
					break;
				case crow::LogLevel::Info:
					console::setColorRGB_f(100, 100, 100);
					std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
					console::resetColor();
					std::cout << "[";
					console::setColor(console::conCol::b_defaultColor, console::conCol::f_cyan);
					std::cout << "Info";
					console::resetColor();
					std::cout << "] ";
					console::setColorRGB_f(255, 255, 255);
					std::cout << message << "\n";
					console::resetColor();
					break;
			#endif
			case crow::LogLevel::Warning:
				console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				console::resetColor();
				std::cout << "[";
				console::setColor(console::conCol::b_defaultColor, console::conCol::f_yellow);
				std::cout << "Warning";
				console::resetColor();
				std::cout << "] ";
				console::setColorRGB_f(255, 255, 255);
				std::cout << message << "\n";
				console::resetColor();
				break;
			case crow::LogLevel::Error:
				console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				console::resetColor();
				std::cout << "[";
				console::setColor(console::conCol::b_defaultColor, console::conCol::f_red);
				std::cout << "Error";
				console::resetColor();
				std::cout << "] ";
				console::setColorRGB_f(255, 255, 255);
				std::cout << message << "\n";
				console::resetColor();
				break;
			case crow::LogLevel::Critical:
				console::setColorRGB_f(100, 100, 100);
				std::cout << (std::string)dateBuffer + " " + timeBuffer << " ";
				console::resetColor();
				console::setColor(console::conCol::b_red, console::conCol::f_white);
				std::cout << "[Critical]";
				console::resetColor();
				console::setColorRGB_f(255, 255, 255);
				std::cout << " " << message << "\n";
				console::resetColor();
				break;
		}
	}
};

namespace Web::WebHandler {

	/*
		Blocking. Returns:
		  true  → full shutdown
		  false → restart requested
	*/
	static bool start(bool skipInit = false) {
		CustomLogger logger;
		crow::logger::setHandler(&logger);

		if (!skipInit) {
			crow::mustache::set_base("./www/");
			crow::mustache::set_global_base("./www/");

			Web::RouteRegister::registerAll(app);
			app.add_static_dir();
		}

		console::writeLog("Starting Web Server...", true);
		console::writeLog("Web Server should be accessible under:", true);
		console::writeLog("#####################", true, 255, 255, 0);
		std::string url = "http://" + (std::string)OSU_TRACKER_WEBSERVER_HOST + ":" + std::to_string(OSU_TRACKER_WEBSERVER_PORT);
		console::writeLog(url, true, 0, 140, 255);
		console::writeLog("#####################", true, 255, 255, 0);

		app.bindaddr(OSU_TRACKER_WEBSERVER_HOST).port(OSU_TRACKER_WEBSERVER_PORT).signal_clear().run(); // blocking

		console::writeLog("Web Server Terminated", true, 255, 0, 0);
		return shutdown_webServer;
	}
}
