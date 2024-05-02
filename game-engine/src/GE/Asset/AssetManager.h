#pragma once

#include "GE/Asset/Asset.h"

#include "GE/Core/Core.h"

#include "GE/Rendering/Renderer/2D/Renderer2D.h"

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
			case Asset::Type::AudioSource:
				return "AudioSource";
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
			if (typeString == "AudioSource")
			{
				return Asset::Type::AudioSource;
			}

			return Asset::Type::None;
		}

		static std::unordered_map<Asset::Type, std::function<Ref<Asset>()>> s_AssetPlaceholders =
		{
			{ Asset::Type::Texture2D, []() { return Renderer2D::GetWhiteTexture(); }}
		};

	}

	using AssetMap = std::map<UUID, Ref<Asset>>;

	class AssetManager
	{
	public:
		virtual ~AssetManager() {};
		virtual Ref<Asset> GetAsset(UUID handle) = 0; // Blocking

		virtual bool HandleExists(UUID handle) = 0;
		virtual bool AssetLoaded(UUID handle) = 0;

		virtual bool AddAsset(UUID handle) = 0;
		virtual bool RemoveAsset(UUID handle) = 0;

		virtual bool SerializeAssets() = 0;
		virtual bool DeserializeAssets() = 0;

	};

}