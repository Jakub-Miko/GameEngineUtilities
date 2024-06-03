#pragma once
#include <string>
#include <fstream>
#include <memory>

class ConfigObject {
public:
	ConfigObject() {}
	virtual std::string GetString(const std::string& name) = 0;
	virtual std::string GetString(int index) = 0;
	virtual bool IsString(const std::string& name) = 0;
	virtual bool IsString(int index) = 0;
	virtual int GetInt(const std::string& name) = 0;
	virtual int GetInt(int index) = 0;
	virtual bool IsInt(const std::string& name) = 0;
	virtual bool IsInt(int index) = 0;
	virtual double GetFloat(const std::string& name) = 0;
	virtual double GetFloat(int index) = 0;
	virtual bool IsFloat(const std::string& name) = 0;
	virtual bool IsFloat(int index) = 0;
	virtual std::shared_ptr<ConfigObject> GetObject(const std::string& name) = 0;
	virtual std::shared_ptr<ConfigObject> GetObject(int index) = 0;
	virtual bool IsObject(const std::string& name) = 0;
	virtual bool IsObject(int index) = 0;
	virtual bool IsArray(const std::string& name) = 0;
	virtual bool IsArray(int index) = 0;
	virtual size_t GetArraySize() = 0;
	virtual bool Exists(const std::string& name) = 0;
	virtual ~ConfigObject() {};
};

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
	std::shared_ptr<ConfigObject> GetObject(const std::string& name);
	bool Exists(const std::string& name);

private:
	std::shared_ptr<ConfigObject> root_object;
	std::string config_path;
	static ConfigManager* instance;
	ConfigManager(const std::string& config_filepath);
	~ConfigManager();
};