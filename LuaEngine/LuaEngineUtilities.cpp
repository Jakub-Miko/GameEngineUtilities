#include "include/LuaEngineUtilities.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <FileManager.h>
#include <json.hpp>

std::string LuaEngineUtilities::ParseScript(std::string script_in, const std::string& hash, bool construction)
{
    std::string json_script;
    std::string script;
    if (script_in[0] == '@') {
        size_t end = script_in.find_first_of(" \t\n", 1);

        if (end == script_in.npos) {
            throw std::runtime_error("Invalid entity file format");
        }

        std::string tag = script_in.substr(1, end - 1);

        if (tag == "Entity") {
            
            auto script_bg = script_in.find(construction ? "@Entity:Construction_Script" : "@Entity:Inline_Script" , 0);
            if (script_bg != script_in.npos) {
                script_bg += strlen(construction ? "@Entity:Construction_Script" : "@Entity:Inline_Script");
                auto end = script_in.find("@Entity", script_bg);
                std::string parsed_script;
                if (end == script_in.npos) {
                    parsed_script = script_in.substr(script_bg, script_in.npos);
                }
                else {
                    parsed_script = script_in.substr(script_bg, end - script_bg);
                }
                script = parsed_script;
            }
        }
        else {
            throw std::runtime_error("This entity descriptor doesn't support scripts");
        }
    }
    else {
        script = script_in;
    }
    
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

std::string LuaEngineUtilities::LoadScript(const std::string& path, bool construction)
{
    
    std::ifstream file_in(FileManager::Get()->GetPath(path));
    if (file_in.is_open()) {
        std::stringstream stream;
        stream << file_in.rdbuf();
        std::string file = stream.str();
        std::string script = ParseScript(file, ScriptHash(path, construction), construction);
        return script;
    }
    else {
        throw std::runtime_error("Script couldn't be opened");
    }
}


std::string LuaEngineUtilities::ScriptHash(std::string script_path, bool construction)
{
    std::string path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(script_path));
    std::replace(path.begin(), path.end(), '/', '_');
    std::replace(path.begin(), path.end(), '.', '_');
    std::replace(path.begin(), path.end(), '#', '_');

    return (construction ? "Construction_" : "Object_") + path;
}
 