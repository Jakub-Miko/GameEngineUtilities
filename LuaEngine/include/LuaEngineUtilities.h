#pragma once

#include <string>

class LuaEngineUtilities {
public:
	static std::string ParseScript(std::string script, const std::string& hash);
	static std::string LoadScript(const std::string& path);
	static std::string ScriptHash(std::string script_path);

};
