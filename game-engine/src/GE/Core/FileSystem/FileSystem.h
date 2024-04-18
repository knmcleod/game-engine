#pragma once

#include "GE/Core/Core.h"
#include "GE/Core/Buffer.h"

#include <filesystem>

namespace GE
{
	class FileSystem
	{
	public:
		static Buffer ReadScriptingBinaryFile(const std::filesystem::path& filePath);

	};

}