#pragma once
#include <fstream>
#include <iostream>
#undef ERROR

#define log(log_string, severity) Logger::Log(log_string, severity, __LINE__, __FILE__)

class Logger {

public:

	enum LogLevel { INFO, WARN, ERROR };
	enum LogDest { STDOUT, FILE };

	// Log auto, takes a string and the severity of the log level and either prints it or tosses it
	static void Log(std::string log_string, LogLevel severity, uint32_t line_number = 0, const char* file_name = nullptr);

	static void set_log_level(LogLevel log_level);
	static void set_log_destination(LogDest log_destination);


private:

	Logger() {};
	~Logger() {
		log_file.close();
	};

	static bool open_log_file();
	static std::ostream& get_stream();

	static LogDest log_destination;
	static LogLevel log_level;
	static std::ofstream log_file;

};
