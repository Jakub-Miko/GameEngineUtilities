#pragma once

#include <string>

class LuaEngineUtilities {
public:
	static std::string ParseScript(std::string script, const std::string& hash, bool construction = false);
	static std::string LoadScript(const std::string& path, bool construction = false);
	static std::string ScriptHash(std::string script_path, bool construction = false);
};
