#pragma once

#include "GE/Asset/Asset.h"

#include <filesystem>
#include <map>

namespace GE
{
	namespace AssetUtils
	{
		const static std::map<std::filesystem::path, Asset::Type> s_AssetExtensions =
		{
			{ ".scene",	Asset::Type::Scene			},
			{ ".jpeg",	Asset::Type::Texture2D		},
			{ ".png",	Asset::Type::Texture2D		},
			{ ".jpg",	Asset::Type::Texture2D		},
			{ ".ttf",	Asset::Type::Font			},
			{ ".wav",	Asset::Type::AudioSource	}
		};

		static Asset::Type AssetTypeFromFileExtension(const std::filesystem::path& extension)
		{
			if (s_AssetExtensions.find(extension) == s_AssetExtensions.end())
			{
				GE_CORE_WARN("Could not determine Asset Type from file extension.\n\tPath:{0}", extension.string());
				return Asset::Type::None;
			}
			return s_AssetExtensions.at(extension);
		}
	}

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
		Asset::Type Type = Asset::Type::None;
		std::filesystem::path FilePath = std::filesystem::path();
		
		AssetStatus Status = AssetStatus::None;

		AssetMetadata()
		{

		}

		AssetMetadata(UUID handle, const std::filesystem::path& filePath = std::filesystem::path())
			: FilePath(filePath), Handle(handle)
		{
			if(!FilePath.empty())
				Type = AssetUtils::AssetTypeFromFileExtension(FilePath.extension());
		}
	};
}
