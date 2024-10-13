#pragma once

#include "AssetMetadata.h"

namespace GE
{
	class AssetRegistry
	{
		friend class AssetManager;
	public:
		AssetRegistry(const std::filesystem::path& filePath = "assetRegistry.gar");
		
		inline void Clear() { m_AssetRegistry.clear(); m_AssetRegistry = std::map<UUID, AssetMetadata>(); }
		inline const std::map<UUID, AssetMetadata>& GetRegistry() { return m_AssetRegistry; }
		inline const std::filesystem::path& GetFilePath() { return m_FilePath; }
		void SetFilePath(const std::filesystem::path& filePath);

		const AssetMetadata& GetAssetMetadata(UUID handle);
		bool AssetExists(UUID handle);

		bool AddAsset(const AssetMetadata& metadata);
		bool RemoveAsset(UUID handle);

	private:
		std::filesystem::path m_FilePath;
		std::map<UUID, AssetMetadata> m_AssetRegistry;
	};
}

