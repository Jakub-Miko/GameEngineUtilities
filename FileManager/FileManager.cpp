#include "FileManager.h"
#include <filesystem>
#include <ConfigManager.h>
#include <fstream>
#include <sstream>

FileManager* FileManager::instance = nullptr;

void FileManager::Init(const FileManager_paths& paths)
{
	if (!instance) {
		instance = new FileManager(paths);
	}
}

void FileManager::Init()
{
	FileManager_paths paths;

	if (!ConfigManager::Get()) {
		throw std::runtime_error("If you don't specify explicit file paths, you  need to initialize ConfigManager before FileManager");
	}

	paths.root_path = FileManager::GetRelativeBinaryPath("/") + ConfigManager::Get()->GetString("root_path");
	paths.local_asset_path = paths.root_path + ConfigManager::Get()->GetString("local_asset_path");
	paths.engine_asset_path = paths.root_path + ConfigManager::Get()->GetString("engine_asset_path");
	paths.render_api_path = paths.root_path + ConfigManager::Get()->GetString("render_api_path");
	paths.temp_path = paths.root_path + ConfigManager::Get()->GetString("temp_path");
	if (!std::filesystem::exists(paths.temp_path)) {
		std::filesystem::create_directory(paths.temp_path);
	}

	if (!instance) {
		instance = new FileManager(paths);
	}
}

void FileManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

FileManager* FileManager::Get()
{
	return instance;
}

std::string FileManager::GetPath(const std::string& path)
{
	auto colon = path.find(':', 0);
	
	
	if (colon != path.npos) {
		auto directive = path.substr(0, colon);
		auto file_path = path.substr(colon+1);
		if (std::isupper(directive[0])) {
			throw std::runtime_error("Invalid path format, first character of directive can't be uppercase. Make sure you are using relative paths");
		}
		if (directive == "asset") {
			return std::filesystem::absolute(std::filesystem::path( GetAssetFilePath(file_path))).generic_string();
		}
		else if(directive == "engine_asset") {
			return std::filesystem::absolute(std::filesystem::path(GetEngineAssetFilePath(file_path))).generic_string();
		}
		else if (directive == "api") {
			return std::filesystem::absolute(std::filesystem::path(GetRenderApiAssetFilePath(file_path))).generic_string();
		}
		else if (directive == "absolute") {
			return std::filesystem::absolute(std::filesystem::path(file_path)).generic_string();
		}
		else if (directive == "temp") {
			return std::filesystem::absolute(std::filesystem::path(GetTempFilePath(file_path))).generic_string();
		}
		else {
			throw std::runtime_error("Unknown directive " + directive);
		}
	}
	else {
		return std::filesystem::absolute(std::filesystem::path(GetRootPath() + std::string(path))).generic_string();
	}



}

std::string FileManager::GetRelativeFilepath(const std::string& path)
{
	return GetRootPath() + path;
}

std::string FileManager::GetRenderApiAssetFilePath(const std::string& path)
{
	return paths.render_api_path + path;
}

std::string FileManager::GetAssetFilePath(const std::string& path)
{
	return paths.local_asset_path + path;
}

std::string FileManager::GetTempFilePath(const std::string& path)
{
	return paths.temp_path + path;
}

std::string FileManager::GetEngineAssetFilePath(const std::string& path)
{
	return paths.engine_asset_path + path;
}

std::string FileManager::GetRelativeFilePath(const std::string& absolute_file_path)
{
	using namespace std::filesystem;
	return relative(path(absolute_file_path), path(""_path)).generic_string();
}

std::string FileManager::GetRootPath()
{
	return paths.root_path;
}

bool FileManager::IsSubPath(const std::string& file_path)
{
	return file_path.find('#') != file_path.npos;
}

std::string FileManager::GetFilePathFromSubPath(const std::string& file_path)
{
	auto fnd = file_path.find('#');
	if (fnd == file_path.npos) {
		return file_path;
	}
	
	return file_path.substr(0, fnd);
}

std::string FileManager::GetFileSectionNameFromSubPath(const std::string& file_path)
{
	auto fnd = file_path.find('#');
	if (fnd == file_path.npos) {
		throw std::runtime_error("This file path doesn't contain a file section");
	}
	fnd += 1;
	auto fnd_end = file_path.find_first_of(" \t\n", fnd);
	if (fnd == file_path.npos) {
		throw std::runtime_error("This file path doesn't contain a file section");
	}
	return file_path.substr(fnd, fnd_end - fnd);

}

std::string FileManager::GetFileSection(const std::string& file_path, const std::string section_name)
{
	std::ifstream file(file_path);
	if (!file.is_open()) {
		throw std::runtime_error("File " + file_path + " could not be opened");
	}
	std::stringstream str_stream;
	str_stream << file.rdbuf();
	file.close();
	return GetFileSectionFromString(str_stream.str(), section_name);
}

void FileManager::InsertOrReplaceSection(std::string& file_string, const std::string& new_section_string, const std::string& section_name)
{
	auto fnd_begin = file_string.find("@Section:" + section_name);
	if (fnd_begin != file_string.npos) {
			

		fnd_begin += std::string("@Section:" + section_name).size();

		auto fnd_end = file_string.find("@EndSection", fnd_begin);
		if (fnd_end == file_string.npos) {
			throw std::runtime_error("Section " + section_name + " not found");
		}
		

		file_string.replace(fnd_begin, fnd_end - fnd_begin, "\n" + new_section_string + "\n");
	}
	else {
		file_string.append("@Section:" + section_name + "\n" + new_section_string + "\n@EndSection");
	}
}

std::string FileManager::GetFileSectionFromString(const std::string& file_string, const std::string section_name)
{
	auto fnd_begin = file_string.find("@Section:" + section_name);
	if (fnd_begin == file_string.npos) {
		throw std::runtime_error("Section " + section_name + " not found");
	} 

	fnd_begin += std::string("@Section:" + section_name).size();

	auto fnd_end = file_string.find("@EndSection", fnd_begin);
	if (fnd_end == file_string.npos) {
		throw std::runtime_error("Section " + section_name + " not found");
	}
	fnd_begin = file_string.find_first_not_of(" \n\t", fnd_begin);

	return file_string.substr(fnd_begin, fnd_end - fnd_begin);

}

std::string FileManager::OpenFile(const std::string& path)
{
	std::string file_path = GetPath(path);
	bool is_subpath = IsSubPath(file_path);
	
	std::ifstream file(is_subpath ? GetFilePathFromSubPath(file_path) : file_path);
	if (!file.is_open()) {
		throw std::runtime_error("File " + file_path + " could not be opened");
	}
	std::stringstream str_stream;
	str_stream << file.rdbuf();
	file.close();
	auto str = str_stream.str();
	if (str.find("@Section") != str.npos) {
		if (is_subpath) {
			return GetFileSectionFromString(str, GetFileSectionNameFromSubPath(file_path));
		}
		else {
			return GetFileSectionFromString(str, "Root");
		}
	}
	else {
		if (is_subpath) {
			throw std::runtime_error("This file isn't partitioned into Sections.");
		}
		else {
			return str;
		}
	}
}

std::string FileManager::OpenFileRaw(const std::string& file_path, SectionList* avaliable_sections)
{
	std::string path = GetPath(file_path);
	bool is_subpath = IsSubPath(path);

	std::ifstream file(is_subpath ? GetFilePathFromSubPath(path) : path);
	if (!file.is_open()) {
		throw std::runtime_error("File " + path + " could not be opened");
	}
	std::stringstream str_stream;
	str_stream << file.rdbuf();
	file.close();
	auto str = str_stream.str();
	if (avaliable_sections) {
		size_t position = 0;
		position = str.find("@Section:", position);
		while (position != str.npos) {
			size_t end_word = 0;
			position += strlen("@Section:");
			end_word = str.find_first_of(" \t\n@", position);
			avaliable_sections->insert(str.substr(position, end_word - position));
			position = end_word;
			position = str.find("@Section:", position);
		}
	}
	return str;
}

std::string FileManager::ResolvePath(const std::string& file_path)
{
	return std::filesystem::absolute(std::filesystem::path(file_path)).generic_string();
}

std::string FileManager::GetPathHash(const std::string& file_path)
{
	std::string path = GetRelativeFilePath(GetPath(file_path));
	std::replace(path.begin(), path.end(), '/', '_');
	std::replace(path.begin(), path.end(), '.', '_');
	std::replace(path.begin(), path.end(), '#', '_');
	return path;

}

std::string FileManager::GetRelativeBinaryPath(const std::string& path)
{
	return std::filesystem::current_path().generic_string() + path;
}

FileManager::FileManager(const FileManager_paths& paths) : paths(paths)
{

}

FileManager::~FileManager()
{

}
