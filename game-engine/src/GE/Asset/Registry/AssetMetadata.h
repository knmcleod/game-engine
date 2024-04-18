#pragma once

#include "GE/Asset/Asset.h"
#include "GE/Asset/AssetUtils.h"

#include <filesystem>

namespace GE
{
	enum class AssetStatus
	{
		None = 0,
		Ready = 1,
		Invalid = 2,
		Loading = 3
	};

	struct AssetMetadata
	{
		UUID Handle = 0;
		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;
		
		AssetStatus Status = AssetStatus::None;

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
