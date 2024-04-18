#include "GE/GEpch.h"

#include "RuntimeAssetManager.h"

#include "Serializer/AssetSerializer.h"

namespace GE
{
	RuntimeAssetManager::RuntimeAssetManager()
	{
		DeserializeAssets("assetPack.gap");
	}

	RuntimeAssetManager::~RuntimeAssetManager()
	{
		SerializeAssets("assetPack.gap");
	}

	Ref<Asset> RuntimeAssetManager::GetAsset(UUID handle)
	{
		if (!HandleExists(handle))
		{
			GE_CORE_ERROR("Runtime Asset Handle does not exist.");
			return nullptr;
		}

		if (AssetLoaded(handle))
		{
			return m_LoadedAssets.at(handle);
		}
		Ref<Asset> asset = nullptr;

		asset = m_AssetPack->GetAsset<Asset>(handle);
		if(asset)
			m_LoadedAssets[handle] = asset;

		return asset;
	}

	bool RuntimeAssetManager::HandleExists(UUID handle)
	{
		return m_AssetPack->HandleExists(handle);
	}

	bool RuntimeAssetManager::AssetLoaded(UUID handle)
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	bool RuntimeAssetManager::SaveAsset(UUID handle)
	{
		if (HandleExists(handle) || !AssetLoaded(handle))
			return false;

		return m_AssetPack->AddAsset(m_LoadedAssets.at(handle));
	}
	
	bool RuntimeAssetManager::RemoveAsset(UUID handle)
	{
		if (!HandleExists(handle) || !AssetLoaded(handle))
			return false;

		return m_AssetPack->RemoveAsset(m_LoadedAssets.at(handle));
	}

	bool RuntimeAssetManager::SerializeAssets(const std::filesystem::path& filePath)
	{
		return m_AssetPack->Serialize(filePath);

	}

	bool RuntimeAssetManager::DeserializeAssets(const std::filesystem::path& filePath)
	{
		return m_AssetPack->Deserialize(filePath);
	}

}