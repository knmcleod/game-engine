#pragma once

#include "GE/Asset/AssetManager.h"
#include "GE/Asset/Registry/AssetRegistry.h"

namespace GE
{
	class EditorAssetManager : public AssetManager
	{
	public:
		EditorAssetManager(const AssetMap& assetMap = AssetMap());
		~EditorAssetManager() override;

		/*
		* Returns nullptr if handle does NOT exist
		* or if asset can not be loaded
		*/
		Ref<Asset> GetAsset(UUID handle) override;
		const AssetMap& GetLoadedAssets() override;

		bool HandleExists(UUID handle) override;
		bool AssetLoaded(UUID handle) override;

		bool AddAsset(Ref<Asset> asset) override;
		bool RemoveAsset(UUID handle) override;

		bool SerializeAssets() override;
		bool DeserializeAssets() override;

		inline Ref<AssetRegistry> GetAssetRegistry() { return m_AssetRegistry; }
		Ref<Asset> GetAsset(const std::filesystem::path& filePath);
		bool AddAsset(const AssetMetadata& metadata);

		const AssetMetadata& GetMetadata(UUID handle);
	private:
		bool SerializeAsset(Ref<Asset> asset, const AssetMetadata& metadata);
		Ref<Asset> DeserializeAsset(const AssetMetadata& metadata);

		bool SerializeScene(Ref<Asset> asset, const AssetMetadata& metadata);

		Ref<Asset> DeserializeScene(const AssetMetadata& metadata);
		Ref<Asset> DeserializeTexture2D(const AssetMetadata& metadata);
		Ref<Asset> DeserializeFont(const AssetMetadata& metadata);
		Ref<Asset> DeserializeAudio(const AssetMetadata& metadata);
		Ref<Asset> DeserializeScript(const AssetMetadata& metadata);
	private:
		Ref<AssetRegistry> m_AssetRegistry;
		AssetMap m_LoadedAssets;

		static AssetMetadata s_NullMetadata;
	};
}

