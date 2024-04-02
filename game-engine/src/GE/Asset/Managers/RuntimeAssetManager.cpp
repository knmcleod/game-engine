#include "GE/GEpch.h"

#include "RuntimeAssetManager.h"

namespace GE
{
	Ref<Asset> RuntimeAssetManager::GetAsset(UUID handle)
	{
		Ref<Asset> asset = nullptr;
		if (HandleExists(handle))
			asset = m_LoadedAssets.at(handle);
		return asset;
	}

	bool RuntimeAssetManager::HandleExists(UUID handle)
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	bool RuntimeAssetManager::AssetLoaded(UUID handle)
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	bool RuntimeAssetManager::SaveAsset(UUID handle)
	{
		return false;
	}
	
	bool RuntimeAssetManager::RemoveAsset(UUID handle)
	{
		return false;
	}

	bool RuntimeAssetManager::SerializeAssets()
	{
		return false;
	}

	bool RuntimeAssetManager::DeserializeAssets()
	{
		return false;
	}

}