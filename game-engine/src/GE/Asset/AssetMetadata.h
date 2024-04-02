#pragma once

#include "GE/Asset/Asset.h"
#include "GE/Asset/AssetUtils.h"

#include <filesystem>

namespace GE
{
	struct AssetMetadata
	{
		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;
		
		AssetMetadata()
		{

		}

		AssetMetadata(const std::filesystem::path& filePath)
			: FilePath(filePath)
		{
			Type = AssetUtils::AssetTypeFromFileExtension(filePath.extension());
		}
	};
}
