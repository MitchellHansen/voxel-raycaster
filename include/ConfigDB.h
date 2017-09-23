#pragma once
#include <string>
#include <filesystem>

class ConfigDB {
	
public:

	ConfigDB();
	~ConfigDB();

	bool init(std::string root_config_path);



private:


};