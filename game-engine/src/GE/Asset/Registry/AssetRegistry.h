#pragma once

#include "AssetMetadata.h"

namespace GE
{
	class AssetRegistry
	{
		friend class AssetSerializer;
		friend class AssetManager;
		friend class EditorAssetManager;
	public:
		AssetRegistry();
		const std::map<UUID, AssetMetadata>& GetRegistry() { return m_AssetRegistry; }

		AssetMetadata& GetAssetMetadata(UUID handle);
		bool AssetExists(UUID handle);
		bool AddAsset(const AssetMetadata& metadata);
		bool RemoveAsset(UUID handle);

	private:
		std::filesystem::path m_FilePath;
		std::map<UUID, AssetMetadata> m_AssetRegistry;
	};
}

