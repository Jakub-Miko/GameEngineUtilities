#include "FileManager.h"
#include "FileManager.h"
#include "FileManager.h"
#include <filesystem>
#include <ConfigManager.h>


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
			return GetAssetFilePath(file_path);
		}
		else if(directive == "engine_asset") {
			return GetEngineAssetFilePath(file_path);
		}
		else if (directive == "api") {
			return GetRenderApiAssetFilePath(file_path);
		}
		else {
			throw std::runtime_error("Unknown directive " + directive);
		}
	}
	else {
		return GetRootPath() + std::string(path);
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

std::string FileManager::GetEngineAssetFilePath(const std::string& path)
{
	return paths.engine_asset_path + path;
}

std::string FileManager::GetRootPath()
{
	return paths.root_path;
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
