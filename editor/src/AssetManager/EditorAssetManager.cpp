#include <GE/GE.h>
#include "EditorAssetManager.h"

#include <GE/Asset/Serializer/AssetSerializer.h>

namespace GE
{

	EditorAssetManager::EditorAssetManager()
	{
		m_AssetRegistry = CreateRef<AssetRegistry>();
		DeserializeAssets("assetRegistry.gar");
	}

	EditorAssetManager::~EditorAssetManager()
	{
		SerializeAssets("assetRegistry.gar");
	}

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
		if (HandleExists(handle))
		{
			if (AssetLoaded(handle))
			{
				return m_LoadedAssets.at(handle);
			}
			else
			{
				AssetMetadata& metadata = GetMetadata(handle);
				Ref<Asset> asset = AssetSerializer::ImportAsset(metadata);
				if (asset)
				{
					m_LoadedAssets[handle] = asset;
					return asset;
				}
			}
		}
		return nullptr;
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

	bool EditorAssetManager::SaveAsset(UUID handle)
	{
		if (HandleExists(handle))
			return false;

		return m_AssetRegistry->AddAsset(GetMetadata(handle));
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

	bool EditorAssetManager::SerializeAssets(const std::filesystem::path& filePath)
	{
		std::filesystem::path path = Project::GetPathToAsset(filePath);
		return m_AssetRegistry->Serialize(path);
	}

	bool EditorAssetManager::DeserializeAssets(const std::filesystem::path& filePath)
	{
		std::filesystem::path path = Project::GetPathToAsset(filePath);
		return m_AssetRegistry->Deserialize(path);
	}

}
