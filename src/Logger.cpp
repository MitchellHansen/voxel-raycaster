#include "Logger.h"

Logger::LogDest Logger::log_destination = LogDest::STDOUT;
Logger::LogLevel Logger::log_level = LogLevel::INFO;
std::ofstream Logger::log_file;

void Logger::Log(std::string log_string, LogLevel severity, uint32_t line_number,const char* file_name) {
	 
	if (severity < log_level)
		return;

	std::ostream &output = get_stream();

	switch (severity) {
		
		case LogLevel::INFO: {
			output << "[INFO]  --> ";
			break;
		}
		case LogLevel::WARN: {
			output << "[WARN]  --> ";
			break;
		}
		case LogLevel::ERROR: {
			output << "[ERROR] --> ";
			break;
		}
		default: {
			output << "";
		}
	}

	output << log_string.c_str();

	if (line_number > 0 && file_name)
		output << " (" << file_name << ":" << line_number << ")" << std::endl;
	else
		output << std::endl;
}

void Logger::set_log_level(LogLevel log_level) {
	Logger::log_level = log_level;
}

void Logger::set_log_destination(LogDest log_destination) {
	Logger::log_destination = log_destination;
}

bool Logger::open_log_file() {

	log_file.open("../log/logfile.txt");

	if (!log_file.is_open()) {
		std::cout << "Wooga Wooga! Can't open the log file for writing!" << std::endl;
		return false;
	} else {
		return true;
	}
}

std::ostream& Logger::get_stream() {

	switch (log_destination) {

		case LogDest::STDOUT: {
			return std::cout;
		}
		case LogDest::FILE: {

			// Fall through if the file isn't open
			if (log_file.is_open())
				return log_file;
		}
		default: {
			return std::cout;
		}
	}
}

