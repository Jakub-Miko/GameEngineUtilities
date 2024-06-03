#pragma once
#include "include/ConfigManager.h"
#include <json.hpp>

class JsonConfigObject : public ConfigObject {
public:

	JsonConfigObject(const nlohmann::json& object);
	virtual std::string GetString(const std::string& name) override;
	virtual std::string GetString(int index) override;
	virtual bool IsString(const std::string& name) override;
	virtual bool IsString(int index) override;
	virtual int GetInt(const std::string& name) override;
	virtual int GetInt(int index) override;
	virtual bool IsInt(const std::string& name) override;
	virtual bool IsInt(int index) override;
	virtual double GetFloat(const std::string& name) override;
	virtual double GetFloat(int index) override;
	virtual bool IsFloat(const std::string& name) override;
	virtual bool IsFloat(int index) override;
	virtual bool Exists(const std::string& name) override;
	virtual std::shared_ptr<ConfigObject> GetObject(int index) override;
	virtual bool IsObject(const std::string& name) override;
	virtual bool IsObject(int index) override;
	virtual std::shared_ptr<ConfigObject> GetArray(const std::string& name) override;
	virtual std::shared_ptr<ConfigObject> GetArray(int index) override;
	virtual bool IsArray(const std::string& name) override;
	virtual bool IsArray(int index) override;
	virtual size_t GetArraySize() override;
	virtual std::shared_ptr<ConfigObject> GetObject(const std::string& name) override;
	virtual ~JsonConfigObject() {}


private:
	nlohmann::json json_object;

};
