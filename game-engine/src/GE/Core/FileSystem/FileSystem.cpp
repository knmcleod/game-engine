#include "GE/GEpch.h"

#include "FileSystem.h"

namespace GE
{
	Buffer FileSystem::ReadBinaryFile(const std::filesystem::path& filePath)
	{
		std::ifstream stream(filePath, std::ios::binary | std::ios::ate);
		if (!stream)
		{
			GE_CORE_ASSERT(false, "Failed to open scripting file.");
			return Buffer();
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = end - stream.tellg();
		if (size == 0)
		{
			GE_CORE_ASSERT(false, "File is empty.");
			return Buffer();
		}

		Buffer buffer(size);
		stream.read(buffer.As<char>(), buffer.Size);
		stream.close();

		return buffer;
	}
}