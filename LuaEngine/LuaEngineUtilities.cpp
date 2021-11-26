#include "include/LuaEngineUtilities.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <FileManager.h>

std::string LuaEngineUtilities::ParseScript(std::string script, const std::string& hash)
{
    size_t pos = 0;
    const char* delim = "( \n\t";
    while (pos != script.npos) {
        auto find_res = script.find("function", pos);
        if (find_res == script.npos) break;
        auto space_it = script.find_first_of(delim, find_res);
        auto begin_second_word = script.find_first_not_of(delim, space_it);
        auto end_second_word = script.find_first_of(delim, begin_second_word);
        std::string replacement = hash + ":" + script.substr(begin_second_word, end_second_word - begin_second_word);
        script.replace(script.begin() + begin_second_word, script.begin() + end_second_word, replacement);
        pos = begin_second_word + replacement.size();
    }
    std::stringstream stream;
    stream << hash << " = {}\n";

    return stream.str() + script;
}

std::string LuaEngineUtilities::LoadScript(const std::string& path)
{
    std::ifstream file_in(FileManager::Get()->GetAssetFilePath(path));
    if (file_in.is_open()) {
        std::stringstream stream;
        stream << file_in.rdbuf();
        std::string file = stream.str();
        std::string script = ParseScript(file, ScriptHash(path));
        return script;
    }
    else {
        throw std::runtime_error("Script couldn't be opened");
    }
}


std::string LuaEngineUtilities::ScriptHash(std::string script_path)
{
    std::replace(script_path.begin(), script_path.end(), '/', '_');
    auto first = script_path.find_first_of('.', 0);

    return "Object_" + script_path.substr(0, first);
}
