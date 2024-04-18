#pragma once

#include "GE/Asset/AssetManager.h"
#include "GE/Asset/Registry/AssetRegistry.h"

#include <map>

namespace GE
{
	class EditorAssetManager : public AssetManager
	{
	public:
		EditorAssetManager();
		~EditorAssetManager();

		inline Ref<AssetRegistry> GetAssetRegistry() { return m_AssetRegistry; }
		AssetMetadata& GetMetadata(UUID handle);

		virtual Ref<Asset> GetAsset(UUID handle) override;
		Ref<Asset> GetAsset(const std::filesystem::path& filePath);

		virtual bool HandleExists(UUID handle) override;
		virtual bool AssetLoaded(UUID handle) override;

		virtual bool SaveAsset(UUID handle) override;
		virtual bool RemoveAsset(UUID handle) override;

		virtual bool SerializeAssets(const std::filesystem::path& filePath) override;
		virtual bool DeserializeAssets(const std::filesystem::path& filePath) override;

	private:
		Ref<AssetRegistry> m_AssetRegistry;
		AssetMap m_LoadedAssets;
	};
}

