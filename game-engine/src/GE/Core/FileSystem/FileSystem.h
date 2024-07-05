#pragma once
#include <string>

namespace GE
{
	class FileSystem
	{
	public:
		// Move to AssetSerializer(?)
		static Buffer ReadScriptingBinaryFile(const std::filesystem::path& filePath);

		/// Returns Empty String if Cancelled
		static std::string LoadFromFileDialog(const char* filter);

		/// Returns Empty String if Cancelled
		static std::string SaveFromFileDialog(const char* filter);
	};

}
