#pragma once
#include <string>
#include <set>

struct FileManager_paths {
	std::string root_path = "Unknown";
	std::string local_asset_path = "Unknown";
	std::string engine_asset_path = "Unknown";
	std::string render_api_path = "Unknown";
	std::string temp_path = "Unknown";
};

using SectionList = typename std::template set<std::string>;

class FileManager {
public:

	enum class FilePathType : char {
		ABSOLUTE_PATH = 0,
		RELATIVE_PATH = 1,
		PREFIXED_PATH = 2
	};
	
	enum class FilePrefixEnum : char {
		SCENE_PREFIX = 0,
		ASSET_PREFIX = 1,
		ENGINE_ASSET_PATH = 2,
		RENDER_API_PATH = 3,
		TEMP_PATH = 4,
		FILE_PREFIX_COUNT
	};
	
	struct FilePrefix {
		std::string prefix;
		std::string relative_path;
		std::string absolute_path;
	};

	FileManager(const FileManager& ref) = delete;
	FileManager(FileManager&& ref) = delete;
	FileManager& operator=(const FileManager& ref) = delete;
	FileManager& operator=(FileManager&& ref) = delete;

	static void Init(const FileManager_paths& paths);
	static void Init(const std::string& root_path);
	static void Shutdown();
	static FileManager* Get();

	std::string GetPath(const std::string& path, bool normalize = false);
	std::string GetPathAbsolute(const std::string& path, bool normalize = false);
	std::string GetPathRelative(const std::string& path, bool normalize = false);
	std::string GetRenderApiAssetFilePath(const std::string& path);
	std::string GetAssetFilePath(const std::string& path);
	std::string GetTempFilePath(const std::string& path);
	std::string GetEngineAssetFilePath(const std::string& path);
	std::string GetRootPath();

	bool IsSubPath(const std::string& file_path);
	std::string GetFilePathFromSubPath(const std::string& file_path);
	std::string GetFileSectionNameFromSubPath(const std::string& file_path);
	std::string GetFileSection(const std::string& file_path, const std::string section_name);
	void InsertOrReplaceSection(std::string& file_string, const std::string& new_section_string, const std::string& section_name);
	std::string GetFileSectionFromString(const std::string& file_string, const std::string section_name);
	std::string OpenFile(const std::string& file_path);
	std::string OpenFileRaw(const std::string& file_path, SectionList* avaliable_sections = nullptr);
	std::string ResolvePath(const std::string& file_path);
	std::string GetPathHash(const std::string& file_path);
	std::string GetLibraryPath(const std::string& library_name);

	static std::string GetWorkDirPath(const std::string& path);

	FilePathType GetFilePathType(const std::string& path, FilePrefixEnum* prefix_type = nullptr, std::string* path_after_prefix = nullptr);

private:
	static FileManager* instance;
	FilePrefix prefix_entries[(int)FilePrefixEnum::FILE_PREFIX_COUNT];
	std::string binary_directory;
	std::string absolute_root_path;
	int max_prefix_size = 15;

	void SetDirectoryPrefixPath(FilePrefixEnum entry, const std::string& absolute_path);

	FileManager(const FileManager_paths& paths);
	~FileManager();
};

inline std::string operator"" _path(const char* in_path, std::size_t) {
	return FileManager::Get()->GetPath(in_path);
}