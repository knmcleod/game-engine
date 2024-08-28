#include "GE/GEpch.h"

#include "RuntimeAssetManager.h"
#include "Serializer/AssetSerializer.h"

namespace GE
{
	RuntimeAssetManager::RuntimeAssetManager(const AssetMap& assetMap /*= AssetMap()*/) : m_LoadedAssets(assetMap)
	{
		m_AssetPack = CreateRef<AssetPack>();
	}

	RuntimeAssetManager::~RuntimeAssetManager()
	{
		m_LoadedAssets.clear();
		m_AssetPack->ClearAllFileData();
	}

	Ref<Asset> RuntimeAssetManager::GetAsset(UUID handle)
	{
		Ref<Asset> asset = nullptr;
		if (!HandleExists(handle))
		{
			GE_CORE_ERROR("Runtime Asset Handle does not exist.");
			return asset;
		}

		if (AssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			const AssetInfo& assetInfo = m_AssetPack->GetAssetInfo(handle);
			asset = AssetSerializer::DeserializeAsset(assetInfo);
			if (asset)
				m_LoadedAssets.at(handle) = asset;
		}

		return asset;
	}

	const AssetMap& RuntimeAssetManager::GetLoadedAssets()
	{
		return m_LoadedAssets;
	}

	bool RuntimeAssetManager::HandleExists(UUID handle)
	{
		return m_AssetPack->HandleExists(handle);
	}

	bool RuntimeAssetManager::AssetLoaded(UUID handle)
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}
	
	bool RuntimeAssetManager::AddAsset(UUID handle)
	{
		if (HandleExists(handle) || !AssetLoaded(handle))
			return false;
		return m_AssetPack->AddAsset(m_LoadedAssets.at(handle), AssetInfo((uint16_t)m_LoadedAssets.at(handle)->GetType()));
	}

	bool RuntimeAssetManager::AddAsset(Ref<Asset> asset)
	{
		if (AssetLoaded(asset->GetHandle()))
		{
			GE_CORE_WARN("Cannot add Asset to Loaded. Asset already in Loaded.");
			return false;
		}

		UUID handle = asset->GetHandle();
		m_LoadedAssets.emplace(handle, asset);
		return true;
	}

	bool RuntimeAssetManager::RemoveAsset(UUID handle)
	{
		if (!HandleExists(handle) || !AssetLoaded(handle))
			return false;

		return m_AssetPack->RemoveAsset(m_LoadedAssets.at(handle));
	}

	bool RuntimeAssetManager::SerializeAssets()
	{
		return AssetSerializer::SerializePack(m_AssetPack);
	}

	bool RuntimeAssetManager::DeserializeAssets()
	{
		return AssetSerializer::DeserializePack(m_AssetPack);
	}

}