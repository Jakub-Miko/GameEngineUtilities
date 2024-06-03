#include "JsonConfigObject.h"

std::string JsonConfigObject::GetString(const std::string& name)
{
	if (json_object[name].is_string()) {
		return json_object[name].get<std::string>();
	}
	else {
		throw std::runtime_error("config property isn't a string or doesn't exist");
	}
}

int JsonConfigObject::GetInt(const std::string& name)
{
	if (json_object[name].is_number_integer()) {
		return json_object[name].get<int>();
	}
	else {
		throw std::runtime_error("config property isn't an integer or doesn't exist");
	}
}

double JsonConfigObject::GetFloat(const std::string& name)
{
	if (json_object[name].is_number_float()) {
		return json_object[name].get<double>();
	}
	else {
		throw std::runtime_error("config property isn't a float or doesn't exist");
	}
}

bool JsonConfigObject::Exists(const std::string& name)
{
	if (json_object.contains(name)) {
		return true;
	}
	else {
		return false;
	}
}

std::shared_ptr<ConfigObject> JsonConfigObject::GetObject(const std::string& name)
{
	if (json_object[name].is_object()) {
		return std::make_shared<JsonConfigObject>(json_object[name]);
	}
	else {
		throw std::runtime_error("config property isn't an object or doesn't exist");
	}
}

std::shared_ptr<ConfigObject> JsonConfigObject::GetArray(const std::string& name)
{
	if (json_object[name].is_array()) {
		return std::make_shared<JsonConfigObject>(json_object[name]);
	}
	else {
		throw std::runtime_error("config property isn't an object or doesn't exist");
	}
}

std::shared_ptr<ConfigObject> JsonConfigObject::GetArray(int index)
{
	if (json_object[index].is_array()) {
		return std::make_shared<JsonConfigObject>(json_object[index]);
	}
	else {
		throw std::runtime_error("config property isn't an object or doesn't exist");
	}
}

std::string JsonConfigObject::GetString(int index)
{
	if (json_object[index].is_string()) {
		return json_object[index].get<std::string>();
	}
	else {
		throw std::runtime_error("config property isn't a string or doesn't exist");
	}
}

bool JsonConfigObject::IsString(const std::string& name)
{
	return json_object[name].is_string();
}

bool JsonConfigObject::IsString(int index)
{
	return json_object[index].is_string();
}

int JsonConfigObject::GetInt(int index)
{
	if (json_object[index].is_number_integer()) {
		return json_object[index].get<int>();
	}
	else {
		throw std::runtime_error("config property isn't an integer or doesn't exist");
	}
}

bool JsonConfigObject::IsInt(const std::string& name)
{
	return json_object[name].is_number_integer();
}

bool JsonConfigObject::IsInt(int index)
{
	return json_object[index].is_number_integer();
}

double JsonConfigObject::GetFloat(int index)
{
	if (json_object[index].is_number_float()) {
		return json_object[index].get<double>();
	}
	else {
		throw std::runtime_error("config property isn't a float or doesn't exist");
	}
}

bool JsonConfigObject::IsFloat(const std::string& name)
{
	return json_object[name].is_number_float();
}

bool JsonConfigObject::IsFloat(int index)
{
	return json_object[index].is_number_float();
}

std::shared_ptr<ConfigObject> JsonConfigObject::GetObject(int index)
{
	if (json_object[index].is_object()) {
		return std::make_shared<JsonConfigObject>(json_object[index]);
	}
	else {
		throw std::runtime_error("config property isn't an object or doesn't exist");
	}
}

bool JsonConfigObject::IsObject(const std::string& name)
{
	return json_object[name].is_object();
}

bool JsonConfigObject::IsObject(int index)
{
	return json_object[index].is_object();
}

bool JsonConfigObject::IsArray(const std::string& name)
{
	return json_object[name].is_array();
}

bool JsonConfigObject::IsArray(int index)
{
	return json_object[index].is_array();
}

size_t JsonConfigObject::GetArraySize()
{
	return json_object.is_array() ? json_object.size() : -1;
}

JsonConfigObject::JsonConfigObject(const nlohmann::json& object) : json_object(object), ConfigObject()
{

}
