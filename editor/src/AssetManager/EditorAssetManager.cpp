#include "EditorAssetManager.h"

#include <GE/Asset/Serializer/AssetSerializer.h>

namespace GE
{

	AssetMetadata& EditorAssetManager::GetMetadata(UUID handle)
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
				AssetMetadata& metadata = GetMetadata(handle);
				metadata.Status = AssetStatus::Loading;
				asset = AssetSerializer::DeserializeAsset(metadata);
				if (asset)
				{
					metadata.Status = AssetStatus::Ready;
					m_LoadedAssets[metadata.Handle] = asset;
					return asset;
				}
				else
				{
					metadata.Status = AssetStatus::Invalid;
				}
			}
		}
		return asset;
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

	bool EditorAssetManager::AddAsset(const AssetMetadata& metadata)
	{
		if(HandleExists(metadata.Handle))
			return false;

		return m_AssetRegistry->AddAsset(metadata);
	}

	bool EditorAssetManager::RemoveAsset(UUID handle)
	{
		if (HandleExists(handle))
		{
			if (AssetLoaded(handle))
				m_LoadedAssets.erase(handle);

			m_AssetRegistry->RemoveAsset(handle);
			return true;
		}
		return false;
	}

	bool EditorAssetManager::SerializeAssets()
	{
		std::filesystem::path path = "assetRegistry.gar";
		if (m_AssetRegistry && m_AssetRegistry->m_FilePath != path)
			path = m_AssetRegistry->m_FilePath;
		return SerializeAssets(path);
	}

	bool EditorAssetManager::DeserializeAssets()
	{
		std::filesystem::path path = "assetRegistry.gar";
		if (m_AssetRegistry && m_AssetRegistry->m_FilePath != path)
			path = m_AssetRegistry->m_FilePath;
		return DeserializeAssets(path);
	}

	bool EditorAssetManager::SerializeAssets(const std::filesystem::path& filePath)
	{
		if (m_AssetRegistry->m_FilePath != filePath)
			m_AssetRegistry->m_FilePath = filePath;

		for (const auto& [handle, metadata] : m_AssetRegistry->GetRegistry())
		{
			if (AssetLoaded(handle))
				AssetSerializer::SerializeAsset(metadata);
		}

		return AssetSerializer::SerializeRegistry(m_AssetRegistry);
	}

	bool EditorAssetManager::DeserializeAssets(const std::filesystem::path& filePath)
	{
		m_AssetRegistry = CreateRef<AssetRegistry>();
		if (!AssetSerializer::DeserializeRegistry(filePath, m_AssetRegistry))
			return false;

		for (const auto& [handle, metadata] : m_AssetRegistry->GetRegistry())
		{
			if (Ref<Asset> asset = AssetSerializer::DeserializeAsset(metadata))
			{
				m_LoadedAssets.emplace(handle, asset);
			}
		}
		return true;
	}

}
