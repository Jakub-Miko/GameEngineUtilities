#include "ConfigManager.h"
#include <json.hpp>
#include <sstream>
#include "include/ConfigManager.h"
#include <stdexcept>
#include "JsonConfigObject.h"

ConfigManager* ConfigManager::instance = nullptr;

ConfigManager::ConfigManager(const std::string& config_filepath) : config_path(config_filepath), root_object()
{
	std::fstream config_file;
	config_file.open(config_filepath);
	if (config_file.is_open()) {
		std::stringstream stream;
		stream << config_file.rdbuf();
		nlohmann::json json;
		stream >> json;
		root_object = std::shared_ptr<JsonConfigObject>(new JsonConfigObject(json));
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

	return root_object->GetString(name);
}

int ConfigManager::GetInt(const std::string& name)
{

	return root_object->GetInt(name);
}

double ConfigManager::GetFloat(const std::string& name)
{

	return root_object->GetFloat(name);
}

bool ConfigManager::Exists(const std::string& name)
{

	return root_object->Exists(name);
}

std::shared_ptr<ConfigObject> ConfigManager::GetObject(const std::string& name) {
	return root_object->GetObject(name);
}

std::shared_ptr<ConfigObject> ConfigManager::GetArray(const std::string& name) {
	return root_object->GetArray(name);
}