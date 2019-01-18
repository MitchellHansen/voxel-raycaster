#pragma once
#include <experimental/filesystem>
#include <iostream>
#include <string>

class ConfigDB {
	
public:

	ConfigDB();
	~ConfigDB();

	bool init(std::string root_config_path);

private:


};
