#include "GE/GEpch.h"

#include "AssetRegistry.h"

namespace GE
{
	AssetRegistry::AssetRegistry()
	{
		m_AssetRegistry = std::map<UUID, AssetMetadata>();
	}

	AssetMetadata& AssetRegistry::GetAssetMetadata(UUID handle)
	{
		return m_AssetRegistry.find(handle)->second;
	}

	bool AssetRegistry::AssetExists(UUID handle)
	{
		if (m_AssetRegistry.find(handle) == m_AssetRegistry.end())
			GE_CORE_WARN("Asset Handle does not exist in Registry.");

		return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool AssetRegistry::AddAsset(const AssetMetadata& metadata)
	{
		if (metadata.Handle == 0)
		{
			GE_CORE_TRACE("Asset Metadata does not exist.");
			return false;
		}

		if (m_AssetRegistry.find(metadata.Handle) != m_AssetRegistry.end())
		{
			GE_CORE_TRACE("Asset already exists in Registry.");
			return false;
		}
		m_AssetRegistry.emplace(metadata.Handle, metadata);
		return true;
	}

	bool AssetRegistry::RemoveAsset(UUID handle)
	{
		if (!AssetExists(handle))
		{
			return false;
		}
		m_AssetRegistry.erase(handle);
		return true;
	}

}
