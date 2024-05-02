#pragma once

#include "GE/Asset/AssetManager.h"
#include "GE/Asset/Registry/AssetRegistry.h"

#include <map>

namespace GE
{
	class EditorAssetManager : public AssetManager
	{
	public:
		EditorAssetManager() = default;
		~EditorAssetManager() override = default;

		Ref<Asset> GetAsset(UUID handle) override;

		bool HandleExists(UUID handle) override;
		bool AssetLoaded(UUID handle) override;

		bool AddAsset(UUID handle) override;
		bool RemoveAsset(UUID handle) override;

		bool SerializeAssets() override;
		bool DeserializeAssets() override;

		inline Ref<AssetRegistry> GetAssetRegistry() { return m_AssetRegistry; }
		Ref<Asset> GetAsset(const std::filesystem::path& filePath);
		bool AddAsset(const AssetMetadata& metadata);
		AssetMetadata& GetMetadata(UUID handle);

		bool SerializeAssets(const std::filesystem::path& filePath);
		bool DeserializeAssets(const std::filesystem::path& filePath);
	private:
		Ref<AssetRegistry> m_AssetRegistry;
		AssetMap m_LoadedAssets;
	};
}

