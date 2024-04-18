#pragma once

#include "GE/Asset/Asset.h"

#include "GE/Core/Core.h"
#include "GE/Core/UUID/UUID.h"

#include "GE/Rendering/Renderer/2D/Renderer2D.h"

namespace GE
{
	static std::unordered_map<AssetType, std::function<Ref<Asset>()>>  s_AssetPlaceholders =
	{
		{ AssetType::Texture2D, []() { return Renderer2D::GetWhiteTexture(); }}
	};

	using AssetMap = std::map<UUID, Ref<Asset>>;

	class AssetManager
	{
	public:
		virtual Ref<Asset> GetAsset(UUID handle) = 0; // Blocking

		virtual bool HandleExists(UUID handle) = 0;
		virtual bool AssetLoaded(UUID handle) = 0;

		virtual bool SaveAsset(UUID handle) = 0;
		virtual bool RemoveAsset(UUID handle) = 0;

		virtual bool SerializeAssets(const std::filesystem::path& filePath) = 0;
		virtual bool DeserializeAssets(const std::filesystem::path& filePath) = 0;
	};

}