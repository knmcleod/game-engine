#include "GE/GEpch.h"

#include "AssetPack.h"

#include "GE/Project/Project.h"

namespace GE
{
    AssetPack::AssetPack(const std::filesystem::path& filePath /*= "assetPack.gap"*/)
    {
        m_File.Path = filePath;
    }

    AssetPack::~AssetPack()
    {
        ClearAllFileData();
    }

    void AssetPack::ClearAllFileData()
    {
        for (auto& [uuid, sceneInfo] : m_File.Index.Scenes)
        {
            sceneInfo.ClearAllData();
        }
    }

    const AssetInfo& AssetPack::GetAssetInfo(UUID handle)
    {
        if (!HandleExists(handle))
        {
            GE_CORE_WARN("Could not get AssetInfo. Handle does not exist.");
            return 0;
        }

        bool found = false;
        UUID sceneHandle = Project::GetConfig().SceneHandle;

        if (sceneHandle)
        {
            auto it = m_File.Index.Scenes.find(sceneHandle);
            if (it != m_File.Index.Scenes.end())
            {
                const SceneInfo& sceneInfo = m_File.Index.Scenes.at(sceneHandle);
                auto assetIt = sceneInfo.m_Assets.find(handle);
                if (assetIt != sceneInfo.m_Assets.end())
                {
                    found = true;
                    return assetIt->second;
                }
            }
        }

        if (!found)
        {
            for (const auto& [sceneHandle, sceneInfo] : m_File.Index.Scenes)
            {
                auto assetIt = sceneInfo.m_Assets.find(handle);
                if (assetIt != sceneInfo.m_Assets.end())
                {
                    return assetIt->second;
                    break;
                }
            }

        }

        GE_CORE_WARN("Could not get AssetInfo. Returning empty AssetInfo.");
        return 0;
    }

    bool AssetPack::HandleExists(UUID handle)
    {
        return m_HandleIndex.find(handle) != m_HandleIndex.end();
    }

    bool AssetPack::AddAsset(Ref<Asset> asset, const AssetInfo& assetInfo)
    {
        if (HandleExists(asset->GetHandle()))
        {
            GE_CORE_WARN("Asset handle already exists in Pack.");
            return false;
        }

        if (asset->GetType() == Asset::Type::Scene)
        {
            UUID sceneHandle = asset->GetHandle();
            m_File.Index.Scenes[sceneHandle] = SceneInfo(*(SceneInfo*)&assetInfo);

            m_HandleIndex.emplace(sceneHandle);
            for (const auto& [uuid, info] : m_File.Index.Scenes.at(sceneHandle).m_Assets)
            {
                m_HandleIndex.emplace(uuid);
            }
        }
        else
        {
            UUID sceneHandle = Project::GetConfig().SceneHandle;
            if (HandleExists(sceneHandle))
            {
                m_File.Index.Scenes.at(sceneHandle).m_Assets[asset->GetHandle()] = AssetInfo(assetInfo);

                m_HandleIndex.emplace(asset->GetHandle());
            }
            else
            {
                GE_CORE_WARN("Cannot add asset to Pack.\n\tProject Scene Handle does not exist in Pack.");
                return false;
            }
        }
		return true;
    }

	bool AssetPack::RemoveAsset(Ref<Asset> asset)
	{
		UUID handle = asset->GetHandle();
		if (asset->GetType() == Asset::Type::Scene && m_File.Index.Scenes.find(handle) != m_File.Index.Scenes.end())
		{
            m_File.Index.Scenes.at(handle).ClearAllData();
			m_File.Index.Scenes.erase(handle);
            m_HandleIndex.erase(handle);
		}
		else
		{
			UUID sceneHandle = Project::GetConfig().SceneHandle;
			if (!sceneHandle || m_File.Index.Scenes.at(sceneHandle).m_Assets.find(handle) == m_File.Index.Scenes.at(sceneHandle).m_Assets.end())
				return false;

            m_File.Index.Scenes.at(sceneHandle).m_Assets.at(handle).DataBuffer.Release();
			m_File.Index.Scenes.at(sceneHandle).m_Assets.erase(handle);
            m_HandleIndex.erase(handle);
		}
		return true;
	}

}
