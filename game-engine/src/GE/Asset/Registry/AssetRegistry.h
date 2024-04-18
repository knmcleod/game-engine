#pragma once

#include "../AssetUtils.h"
#include "AssetMetadata.h"
#include "GE/Core/UUID/UUID.h"

namespace GE
{
	class AssetRegistry
	{
	public:
		AssetRegistry();
		const std::map<UUID, AssetMetadata>& GetRegistry() { return m_AssetRegistry; }

		AssetMetadata& GetAssetMetadata(UUID handle);
		bool AssetExists(UUID handle);
		bool AddAsset(const AssetMetadata& metadata);
		bool RemoveAsset(UUID handle);

		bool Serialize(const std::filesystem::path& filePath);
		bool Deserialize(const std::filesystem::path& filePath);
	private:
		std::map<UUID, AssetMetadata> m_AssetRegistry;

	};
}

