#include "EditorAssetManager.h"

#include <GE/Asset/Serializer/AssetSerializer.h>

namespace GE
{
	EditorAssetManager::EditorAssetManager(const AssetMap& assetMap /*= AssetMap()*/) : m_LoadedAssets(assetMap)
	{
		m_AssetRegistry = CreateRef<AssetRegistry>();
	}

	EditorAssetManager::~EditorAssetManager()
	{
		m_LoadedAssets.clear();
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(UUID handle)
	{
		static AssetMetadata s_NullMetadata;
		if (!HandleExists(handle))
			return s_NullMetadata;
		return m_AssetRegistry->GetAssetMetadata(handle);
	}

	/*
	* Returns nullptr if handle does NOT exist
	* or if asset can not be loaded
	*/
	Ref<Asset> EditorAssetManager::GetAsset(UUID handle)
	{
		Ref<Asset> asset = nullptr;
		if (HandleExists(handle))
		{
			if (AssetLoaded(handle))
			{
				return m_LoadedAssets.at(handle);
			}
			else
			{
				return LoadAsset(handle);
			}
		}
		return asset;
	}

	const AssetMap& EditorAssetManager::GetLoadedAssets()
	{
		return m_LoadedAssets;
	}

	Ref<Asset> EditorAssetManager::GetAsset(const std::filesystem::path& filePath)
	{
		UUID assetHandle = 0;
		const auto& registry = m_AssetRegistry->GetRegistry();
		for (const auto& [handle, metadata] : registry)
		{
			if (metadata.FilePath == filePath)
				assetHandle = handle;
		}

		return GetAsset(assetHandle);
	}

	bool EditorAssetManager::HandleExists(UUID handle)
	{
		return m_AssetRegistry->AssetExists(handle);
	}

	bool EditorAssetManager::AssetLoaded(UUID handle)
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	bool EditorAssetManager::AddAsset(UUID handle)
	{
		AssetMetadata metadata(handle);
		return AddAsset(metadata);
	}

	bool EditorAssetManager::AddAsset(Ref<Asset> asset)
	{
		if (AssetLoaded(asset->GetHandle()))
		{
			GE_WARN("Cannot add Asset to Loaded. Asset already in Loaded.");
			return false;
		}
		m_LoadedAssets.emplace(asset->GetHandle(), asset);
		return true;
	}

	bool EditorAssetManager::AddAsset(const AssetMetadata& metadata)
	{
		return m_AssetRegistry->AddAsset(metadata);
	}

	bool EditorAssetManager::RemoveAsset(UUID handle)
	{
		if (AssetLoaded(handle))
			m_LoadedAssets.erase(handle);

		if (!HandleExists(handle))
			return false;

		return m_AssetRegistry->RemoveAsset(handle);
	}

	Ref<Asset> EditorAssetManager::LoadAsset(const AssetMetadata& metadata)
	{
		if (AssetLoaded(metadata.Handle))
			return m_LoadedAssets.at(metadata.Handle);

		Ref<Asset> asset = nullptr;
		if (asset = AssetSerializer::DeserializeAsset(m_AssetRegistry->GetAssetMetadata(metadata.Handle)))
		{
			AddAsset(asset);
		}
		return asset;
	}

	bool EditorAssetManager::SerializeAssets()
	{
		for (const auto& [handle, metadata] : m_AssetRegistry->GetRegistry())
		{
			if (AssetLoaded(handle))
				AssetSerializer::SerializeAsset(m_LoadedAssets.at(handle), metadata);
		}

		return AssetSerializer::SerializeRegistry(m_AssetRegistry);
	}

	bool EditorAssetManager::DeserializeAssets()
	{
		if (!AssetSerializer::DeserializeRegistry(m_AssetRegistry))
			return false;

		for (const auto& [handle, metadata] : m_AssetRegistry->GetRegistry())
		{
			LoadAsset(metadata);
		}
		return true;
	}

	bool EditorAssetManager::SerializeAssets(const std::filesystem::path& filePath)
	{
		m_AssetRegistry->SetFilePath(filePath);
		return SerializeAssets();

	}

	bool EditorAssetManager::DeserializeAssets(const std::filesystem::path& filePath)
	{
		m_AssetRegistry->SetFilePath(filePath);
		return DeserializeAssets();
	}

}
