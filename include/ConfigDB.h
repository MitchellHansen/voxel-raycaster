#pragma once
#include <string>
#include <experimental/filesystem>

class ConfigDB {
	
public:

	ConfigDB();
	~ConfigDB();

	bool init(std::string root_config_path);

private:


};
