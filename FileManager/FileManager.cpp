#include "FileManager.h"
#include <filesystem>

FileManager* FileManager::instance = nullptr;

void FileManager::Init()
{
	if (!instance) {
		instance = new FileManager();
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

std::string FileManager::GetRelativeFilepath(const std::string& path)
{
	return GetRootPath() + "/" + path;
}

std::string FileManager::GetRenderApiAssetFilePath(const std::string& path)
{
	return GetRootPath() + render_api_assets_filepath + "/" + path;
}

std::string FileManager::GetAssetFilePath(const std::string& path)
{
	return GetRootPath() + "/assets/" + path;
}

std::string FileManager::GetRootPath()
{
	return std::filesystem::current_path().parent_path().generic_string();
}

FileManager::FileManager()
{
#if RENDER_API == OpenGL
	render_api_assets_filepath = "/assets/OpenGL";
#else 
	render_api_assets_filepath = "/assets";
#endif
}

FileManager::~FileManager()
{

}
