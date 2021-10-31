#include "ConfigManager.h"
#include <dependencies/json.hpp>
#include <sstream>
#include "include/ConfigManager.h"
#include <stdexcept>


ConfigManager* ConfigManager::instance = nullptr;

ConfigManager::ConfigManager(const std::string& config_filepath) : config_file()
{
	std::fstream config_file;
	config_file.open(config_filepath);
	if (config_file.is_open()) {
		std::stringstream stream;
		stream << config_file.rdbuf();
		config_string = stream.str();
		config_file.close();
	}
	else {
		throw std::runtime_error("Config file could not be opened");
	}
}

ConfigManager::~ConfigManager()
{

}

void ConfigManager::Init(const std::string& config_filepath)
{
	if (!instance) {
		instance = new ConfigManager(config_filepath);
	}
}

void ConfigManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

ConfigManager* ConfigManager::Get()
{
	return instance;
}

std::string ConfigManager::GetString(const std::string& name)
{
	std::stringstream stream(config_string);
	nlohmann::json json;
	stream >> json;
	if (json[name].is_string()) {
		return json[name].get<std::string>();
	}
	else {
		throw std::runtime_error("config property isn't a string or doesn't exist");
	}
}

int ConfigManager::GetInt(const std::string& name)
{
	std::stringstream stream(config_string);
	nlohmann::json json;
	stream >> json;
	if (json[name].is_number_integer()) {
		return json[name].get<int>();
	}
	else {
		throw std::runtime_error("config property isn't an int or doesn't exist");
	}
}

double ConfigManager::GetFloat(const std::string& name)
{
	std::stringstream stream(config_string);
	nlohmann::json json;
	stream >> json;
	if (json[name].is_number_float()) {
		return json[name].get<double>();
	}
	else {
		throw std::runtime_error("config property isn't a float or doesn't exist");
	}
}
