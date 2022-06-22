#pragma once
#include <string>

struct FileManager_paths {
	std::string root_path = "Unknown";
	std::string local_asset_path = "Unknown";
	std::string engine_asset_path = "Unknown";
	std::string render_api_path = "Unknown";
};

class FileManager {
public:

	FileManager(const FileManager& ref) = delete;
	FileManager(FileManager&& ref) = delete;
	FileManager& operator=(const FileManager& ref) = delete;
	FileManager& operator=(FileManager&& ref) = delete;

	static void Init(const FileManager_paths& paths);
	static void Init();
	static void Shutdown();
	static FileManager* Get();

	std::string GetPath(const std::string& path);
	std::string GetRelativeFilepath(const std::string& path);
	std::string GetRenderApiAssetFilePath(const std::string& path);
	std::string GetAssetFilePath(const std::string& path);
	std::string GetEngineAssetFilePath(const std::string& path);
	std::string GetRelativeFilePath(const std::string& absolute_file_path);
	std::string GetRootPath();

	static std::string GetRelativeBinaryPath(const std::string& path);

private:
	static FileManager* instance;
	FileManager_paths paths;
	FileManager(const FileManager_paths& paths);
	~FileManager();
};

inline std::string operator"" _path(const char* in_path, std::size_t) {
	return FileManager::Get()->GetPath(in_path);
}