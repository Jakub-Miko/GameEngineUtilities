#include "FileManager.h"
#include <filesystem>
#include <ConfigManager.h>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <sstream>

#include "../../dependencies/glm/glm/gtx/dual_quaternion.hpp"

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

	paths.root_path = FileManager::GetWorkDirPath("/") + ConfigManager::Get()->GetString("root_path");
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

std::string FileManager::GetPath(const std::string& path, bool normalize)
{
	FilePrefixEnum prefix_type;
	std::string path_portion = "";
	auto type = GetFilePathType(path, &prefix_type, &path_portion);

	std::string output_path = "";

	switch (type)
	{
	case FilePathType::ABSOLUTE_PATH:
		output_path = path;
		break;
	case FilePathType::RELATIVE_PATH :
		output_path = path;
		break;
	case FilePathType::PREFIXED_PATH :
		output_path = prefix_entries[(int)prefix_type].relative_path + path_portion;
		break;
	default:
		throw std::runtime_error("Invalid filepath type in path: " + path + "\n");
	}

	if(normalize) {
		return std::filesystem::path(output_path).lexically_normal().generic_string();
	} else {
		return output_path;
	}
}

std::string FileManager::GetPathAbsolute(const std::string &path, bool normalize)
{
    FilePrefixEnum prefix_type;
	std::string path_portion = "";
	auto type = GetFilePathType(path, &prefix_type);

	std::string output_path = "";

	switch (type)
	{
	case FilePathType::ABSOLUTE_PATH:
		output_path = path;
		break;
	case FilePathType::RELATIVE_PATH :
		output_path = GetRootPath() + path;
		break;
	case FilePathType::PREFIXED_PATH :
		output_path = prefix_entries[(int)prefix_type].absolute_path + path_portion;
		break;
	default:
		throw std::runtime_error("Invalid filepath type in path: " + path + "\n");
	}

	if(normalize) {
		return std::filesystem::path(output_path).lexically_normal().generic_string();
	} else {
		return output_path;
	}
}

std::string FileManager::GetPathRelative(const std::string &path, bool normalize)
{
    	FilePrefixEnum prefix_type;
	std::string path_portion = "";
	auto type = GetFilePathType(path, &prefix_type);

	std::string output_path = "";

	switch (type)
	{
	case FilePathType::ABSOLUTE_PATH:
		output_path = std::filesystem::path(path).lexically_relative(GetRootPath()).generic_string();
		break;
	case FilePathType::RELATIVE_PATH :
		output_path = path;
		break;
	case FilePathType::PREFIXED_PATH :
		output_path = prefix_entries[(int)prefix_type].relative_path + path_portion;
		break;
	default:
		throw std::runtime_error("Invalid filepath type in path: " + path + "\n");
	}

	if(normalize) {
		return std::filesystem::path(output_path).lexically_normal().generic_string();
	} else {
		return output_path;
	}
}

std::string FileManager::GetRenderApiAssetFilePath(const std::string& path)
{
	return prefix_entries[(int)FilePrefixEnum::RENDER_API_PATH].relative_path + path;
}

std::string FileManager::GetAssetFilePath(const std::string& path)
{
	return prefix_entries[(int)FilePrefixEnum::ASSET_PREFIX].relative_path + path;
}

std::string FileManager::GetTempFilePath(const std::string& path)
{
	return prefix_entries[(int)FilePrefixEnum::TEMP_PATH].relative_path + path;
}

std::string FileManager::GetEngineAssetFilePath(const std::string& path)
{
	return prefix_entries[(int)FilePrefixEnum::ENGINE_ASSET_PATH].relative_path + path;
}

std::string FileManager::GetRootPath()
{
	return absolute_root_path;
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
	std::string path = GetPath(file_path, true);
	std::replace(path.begin(), path.end(), '/', '_');
	std::replace(path.begin(), path.end(), '.', '_');
	std::replace(path.begin(), path.end(), '#', '_');
	return path;
}

std::string FileManager::GetLibraryPath(const std::string& library_name)
{
#ifdef UNIX
	return binary_directory + "lib" + library_name + ".so";
#elif defined(WIN32)
	return binary_directory + library_name + ".dll";
#else 
	static_assert(false, "Only Linux And Windows is currently supported");
#endif
}

std::string FileManager::GetWorkDirPath(const std::string& path)
{
	return std::filesystem::current_path().generic_string() + path;
}

FileManager::FilePathType FileManager::GetFilePathType(const std::string& path, FileManager::FilePrefixEnum* prefix_type, std::string* path_after_prefix)
{
    if(path.empty()) {
		return FilePathType::RELATIVE_PATH;
	}

	if(path[0] == '/') {
#ifdef WINDOWS
        throw std::runtime_error("Linux format absolute paths cannot be used on windows.\n");
#elif defined LINUX
		return FilePathType::ABSOLUTE_PATH;
#else
		static_assert("Invalid Platform");
#endif
	}

	int offset = -1;
	for(int i = 0; i < max_prefix_size; i++) {
		if(path[i] == ':') {
			offset = i;
			break;
		}
	}

	if(offset == -1) {
		return FilePathType::RELATIVE_PATH;
	}

	if(offset == 1) {
#ifdef WINDOWS
		return FilePathType::ABSOLUTE_PATH;
#elif defined LINUX
		throw std::runtime_error("Windows format absolute paths cannot be used on linux.\n");
#else
		static_assert("Invalid Platform");
#endif
	} 

	for(int i = 0; i < (int)FilePrefixEnum::FILE_PREFIX_COUNT; i++) {
		auto& entry = prefix_entries[i];
		if(entry.prefix.size() != offset) {
			continue;
		}
		auto prefix = path.substr(0, offset);
		if(entry.prefix == prefix) {
			if(prefix_type) {
				*prefix_type = (FilePrefixEnum)i; 
			}
			if(path_after_prefix) {
				*path_after_prefix = path.substr(offset + 1);
			}
			return FilePathType::PREFIXED_PATH;
		}
	}

	throw std::runtime_error("Invalid file prefix.");

}

void FileManager::SetDirectoryPrefixPath(FilePrefixEnum entry, const std::string &absolute_path)
{
	auto normalized = std::filesystem::path(absolute_path).lexically_normal().generic_string();
	prefix_entries[(int)entry].absolute_path = normalized + "/";
	prefix_entries[(int)entry].relative_path = std::filesystem::relative(normalized, GetRootPath()).generic_string() + "/";
}

FileManager::FileManager(const FileManager_paths &paths)
{
	binary_directory = std::filesystem::current_path().generic_string() + "/"; //Save the initial launch working directory as the binary directory
	std::filesystem::current_path(paths.root_path); 

	absolute_root_path = std::filesystem::weakly_canonical(std::filesystem::absolute(paths.root_path)).generic_string() + "/";

	prefix_entries[(int)FilePrefixEnum::SCENE_PREFIX].prefix = "scene";
	SetDirectoryPrefixPath(FilePrefixEnum::SCENE_PREFIX, paths.root_path);

	prefix_entries[(int)FilePrefixEnum::ASSET_PREFIX].prefix = "asset";
	SetDirectoryPrefixPath(FilePrefixEnum::ASSET_PREFIX, paths.local_asset_path);

	prefix_entries[(int)FilePrefixEnum::ENGINE_ASSET_PATH].prefix = "engine_asset";
	SetDirectoryPrefixPath(FilePrefixEnum::ENGINE_ASSET_PATH, paths.engine_asset_path);

	prefix_entries[(int)FilePrefixEnum::RENDER_API_PATH].prefix = "api";
	SetDirectoryPrefixPath(FilePrefixEnum::RENDER_API_PATH, paths.render_api_path);

    prefix_entries[(int)FilePrefixEnum::TEMP_PATH].prefix = "temp";
	SetDirectoryPrefixPath(FilePrefixEnum::TEMP_PATH, paths.temp_path);

	max_prefix_size = prefix_entries[0].prefix.size();
	for(int i = 1; i < (int)FilePrefixEnum::FILE_PREFIX_COUNT; i++) {
		max_prefix_size = std::max(max_prefix_size, (int)prefix_entries[i].prefix.size());
	}
	max_prefix_size++; //Account for the colon

}

FileManager::~FileManager()
{

}
