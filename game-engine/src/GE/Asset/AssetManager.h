#pragma once

#include "GE/Asset/Assets/Asset.h"

#include "GE/Core/Core.h"

namespace GE
{
	namespace AssetUtils
	{
		static const std::string AssetTypeToString(Asset::Type type)
		{
			switch (type)
			{
			case Asset::Type::Scene:
				return "Scene";
				break;
			case Asset::Type::Texture2D:
				return "Texture2D";
				break;
			case Asset::Type::Font:
				return "Font";
				break;
			case Asset::Type::Audio:
				return "Audio";
				break;
			case Asset::Type::Script:
				return "Script";
				break;
			}

			return "None";

		}

		static Asset::Type AssetTypeFromString(const std::string& typeString)
		{
			if (typeString == "Scene")
			{
				return Asset::Type::Scene;
			}
			if (typeString == "Texture2D")
			{
				return Asset::Type::Texture2D;
			}
			if (typeString == "Font")
			{
				return Asset::Type::Font;
			}
			if (typeString == "Audio")
			{
				return Asset::Type::Audio;
			}
			if (typeString == "Script")
			{
				return Asset::Type::Script;
			}

			return Asset::Type::None;
		}

	}

	using AssetMap = std::map<UUID, Ref<Asset>>;

	class AssetManager
	{
	public:
		virtual ~AssetManager() = default;

		virtual void InvalidateAssets() = 0;

		virtual Ref<Asset> GetAsset(UUID handle) = 0;
		virtual const AssetMap& GetLoadedAssets() = 0;

		virtual bool HandleExists(UUID handle) = 0;
		virtual bool AssetLoaded(UUID handle) = 0;

		//virtual bool AddAsset(UUID handle) = 0;
		virtual bool AddAsset(Ref<Asset> asset) = 0;
		virtual bool RemoveAsset(UUID handle) = 0;

		virtual bool SerializeAssets() = 0;
		virtual bool DeserializeAssets() = 0;

	};

}