#pragma once

#include "AssetManager.h"
#include "Pack/AssetPack.h"

namespace GE
{
	class RuntimeAssetManager : public AssetManager
	{
	public:
		RuntimeAssetManager();
		~RuntimeAssetManager();

		virtual Ref<Asset> GetAsset(UUID handle) override;
		virtual bool HandleExists(UUID handle) override;
		virtual bool AssetLoaded(UUID handle) override;

		virtual bool SaveAsset(UUID handle) override;
		virtual bool RemoveAsset(UUID handle) override;

		virtual bool SerializeAssets(const std::filesystem::path& filePath) override;
		virtual bool DeserializeAssets(const std::filesystem::path& filePath) override;
	private:
		Ref<AssetPack> m_AssetPack;

		AssetMap m_LoadedAssets;
	};

}