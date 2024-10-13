#pragma once

#include "GE/Asset/Assets/Asset.h"

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
			{ ".wav",	Asset::Type::Audio			},
			{ ".cs",	Asset::Type::Script			}
			// { ".mp3",	Asset::Type::Audio			},
			// { ".ogg",	Asset::Type::Audio			},

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

	struct AssetMetadata
	{
		UUID Handle = 0;
		Asset::Type Type = Asset::Type::None;
		std::filesystem::path FilePath = std::filesystem::path();
		
		Asset::Status Status = Asset::Status::None;

		AssetMetadata() = default;

		AssetMetadata(UUID handle, const std::filesystem::path& filePath)
			: FilePath(filePath), Handle(handle)
		{
			if (!FilePath.empty())
			{
				Type = AssetUtils::AssetTypeFromFileExtension(FilePath.extension());
			}
		}
	};
}
