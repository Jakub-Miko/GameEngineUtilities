#pragma once
#include <string>

class FileManager {
public:

	FileManager(const FileManager& ref) = delete;
	FileManager(FileManager&& ref) = delete;
	FileManager& operator=(const FileManager& ref) = delete;
	FileManager& operator=(FileManager&& ref) = delete;

	static void Init();
	static void Shutdown();
	static FileManager* Get();

	std::string GetRelativeFilepath(const std::string& path);
	std::string GetRenderApiAssetFilePath(const std::string& path);
	std::string GetAssetFilePath(const std::string& path);
	std::string GetRootPath();

private:
	static FileManager* instance;
	std::string render_api_assets_filepath;
	FileManager();
	~FileManager();
};