#pragma once
#include <string>

struct FileManager_paths {
	std::string root_path = "Unknown";
	std::string local_asset_path = "Unknown";
	std::string engine_asset_path = "Unknown";
	std::string render_api_path = "Unknown";
	std::string temp_path = "Unknown";
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
	std::string GetTempFilePath(const std::string& path);
	std::string GetEngineAssetFilePath(const std::string& path);
	std::string GetRelativeFilePath(const std::string& absolute_file_path);
	std::string GetRootPath();

	bool IsSubPath(const std::string& file_path);
	std::string GetFilePathFromSubPath(const std::string& file_path);
	std::string GetFileSectionNameFromSubPath(const std::string& file_path);
	std::string GetFileSection(const std::string& file_path, const std::string section_name);
	void InsertOrReplaceSection(std::string& file_string, const std::string& new_section_string, const std::string& section_name);
	std::string GetFileSectionFromString(const std::string& file_string, const std::string section_name);
	std::string OpenFile(const std::string& file_path);
	std::string OpenFileRaw(const std::string& file_path);
	std::string ResolvePath(const std::string& file_path);

	std::string GetPathHash(const std::string& file_path);

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