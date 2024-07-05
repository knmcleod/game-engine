#pragma once

#include "GE/Asset/AssetManager.h"
#include "GE/Asset/Registry/AssetRegistry.h"

namespace GE
{
	class EditorAssetManager : public AssetManager
	{
	public:
		EditorAssetManager();
		~EditorAssetManager() override = default;

		Ref<Asset> GetAsset(UUID handle) override;
		const AssetMap& GetLoadedAssets() override;

		bool HandleExists(UUID handle) override;
		bool AssetLoaded(UUID handle) override;

		bool AddAsset(UUID handle) override;
		bool AddAsset(Ref<Asset> asset) override;
		bool RemoveAsset(UUID handle) override;

		bool SerializeAssets() override;
		bool DeserializeAssets() override;

		inline Ref<AssetRegistry> GetAssetRegistry() { return m_AssetRegistry; }
		Ref<Asset> GetAsset(const std::filesystem::path& filePath);
		bool AddAsset(const AssetMetadata& metadata);

		const AssetMetadata& GetMetadata(UUID handle);
	private:
		Ref<Asset> LoadAsset(const AssetMetadata& metadata);

		bool SerializeAssets(const std::filesystem::path& filePath);
		bool DeserializeAssets(const std::filesystem::path& filePath);
	private:
		Ref<AssetRegistry> m_AssetRegistry;
		AssetMap m_LoadedAssets;
	};
}

