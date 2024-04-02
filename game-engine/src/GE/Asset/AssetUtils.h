#pragma once

#include "Asset.h"

#include <string>
#include <filesystem>
#include <map>

namespace GE
{
	enum class AssetType
	{
		None = 0,
		Scene,
		Texture2D
	};

	namespace AssetUtils
	{
		static const std::string AssetTypeToString(AssetType type)
		{
			switch (type)
			{
			case AssetType::Scene:
				return "Scene";
				break;
			case AssetType::Texture2D:
				return "Texture2D";
				break;
			}
			return "None";

		}

		static AssetType AssetTypeFromString(const std::string& typeString)
		{
			if (typeString == "Scene")
			{
				return AssetType::Scene;
			}
			if (typeString == "Texture2D")
			{
				return AssetType::Texture2D;
			}

			return AssetType::None;
		}

		const static std::map<std::filesystem::path, AssetType> s_AssetExtensions =
		{
			{ ".scene",	AssetType::Scene		},
			{ ".png",	AssetType::Texture2D	},
			{ ".jpg",	AssetType::Texture2D	},
			{ ".jpeg",	AssetType::Texture2D	}
		};

		static AssetType AssetTypeFromFileExtension(const std::filesystem::path& extension)
		{
			if (s_AssetExtensions.find(extension) == s_AssetExtensions.end())
			{
				return AssetType::None;
			}
			return s_AssetExtensions.at(extension);
		}
	}
}
