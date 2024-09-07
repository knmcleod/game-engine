#include "GE/GEpch.h"

#include "AssetRegistry.h"

namespace GE
{
	AssetRegistry::AssetRegistry(const std::filesystem::path& filePath)
	{
		m_FilePath = filePath;
		m_AssetRegistry = std::map<UUID, AssetMetadata>();
	}

	void AssetRegistry::SetFilePath(const std::filesystem::path& filePath)
	{
		if (!filePath.empty() && filePath != m_FilePath)
			m_FilePath = filePath;
	}

	const AssetMetadata& AssetRegistry::GetAssetMetadata(UUID handle)
	{
		return m_AssetRegistry.find(handle)->second;
	}

	bool AssetRegistry::AssetExists(UUID handle)
	{
		return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool AssetRegistry::AddAsset(const AssetMetadata& metadata)
	{
		if (metadata.Handle == 0)
		{
			GE_CORE_WARN("Asset Metadata does not exist.");
			return false;
		}

		if (AssetExists(metadata.Handle))
		{
			GE_CORE_WARN("Asset already exists in Registry.");
			return false;
		}
		m_AssetRegistry.emplace(metadata.Handle, metadata);
		return true;
	}

	bool AssetRegistry::RemoveAsset(UUID handle)
	{
		if (!AssetExists(handle))
			return false;

		m_AssetRegistry.erase(handle);
		return true;
	}

}
