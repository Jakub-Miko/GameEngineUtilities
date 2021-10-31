#pragma once
#include <string>
#include <fstream>

class ConfigManager {
public:
	ConfigManager(const ConfigManager& ref) = delete;
	ConfigManager(ConfigManager&& ref) = delete;
	ConfigManager& operator=(const ConfigManager& ref) = delete;
	ConfigManager& operator=(ConfigManager&& ref) = delete;

	static void Init(const std::string& config_filepath = "config.json");
	static void Shutdown();
	static ConfigManager* Get();

	std::string GetString(const std::string& name);
	int GetInt(const std::string& name);
	double GetFloat(const std::string& name);

private:
	std::fstream config_file;
	std::string config_string;
	static ConfigManager* instance;
	ConfigManager(const std::string& config_filepath);
	~ConfigManager();
};