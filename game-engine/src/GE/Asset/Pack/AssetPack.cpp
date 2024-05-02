#include "GE/GEpch.h"

#include "AssetPack.h"

#include "GE/Project/Project.h"

#include "GE/Asset/Serializer/AssetSerializer.h"

namespace GE
{
    template<typename T>
    Ref<T> AssetPack::GetAsset(UUID handle)
    {
        const AssetPackFile::AssetInfo* assetInfo = nullptr;

        bool found = false;
		UUID sceneHandle = Project::GetActive()->GetConfig().SceneHandle;

        if (sceneHandle)
        {
            auto it = m_File.Index.Scenes.find(sceneHandle);
            if (it != m_File.Index.Scenes.end())
            {
                const AssetPackFile::SceneInfo& sceneInfo = m_File.Index.Scenes.at(sceneHandle);
                auto assetIt = sceneInfo.Assets.find(handle);
                if (assetIt != sceneInfo.Assets.end())
                {
                    found = true;
                    assetInfo = &assetIt-second;
                }
            }
        }

        if (!found)
        {
            for (const auto& [sceneHandle, sceneInfo] : m_File.Index.Scenes)
            {
                auto assetIt = sceneInfo.Assets.find(handle);
                if (assetIt != sceneInfo.Assets.end())
                {
                    assetInfo = &assetIt-second;
                    break;
                }
            }

            if (!assetInfo)
                return nullptr;
        }

		Ref<T> asset = AssetSerializer::DeserializeAsset(assetInfo);
        if (!asset)
            return nullptr;

        return asset;
    }

    uint64_t AssetPack::GetBuildVersion() const
    {
        return m_File.Header.BuildVersion;
    }

    bool AssetPack::HandleExists(UUID handle)
    {
        return m_HandleIndex.find(handle) != m_HandleIndex.end();
    }

    bool AssetPack::AddAsset(Ref<Asset> asset)
    {
        UUID sceneHandle = Project::GetActive()->GetConfig().SceneHandle;
		if (!sceneHandle)
			return false;

		AssetPack::File::AssetInfo assetInfo;
		assetInfo.Type = (uint16_t)asset->GetType();
        m_File.Index.Scenes.at(sceneHandle).Assets.emplace(asset->GetHandle(), assetInfo);
		return true;
    }

	bool AssetPack::RemoveAsset(Ref<Asset> asset)
	{
		UUID handle = asset->GetHandle();
		if (asset->GetType() == Asset::Type::Scene && m_File.Index.Scenes.find(handle) != m_File.Index.Scenes.end())
		{
			m_File.Index.Scenes.erase(handle);
			return true;
		}
		else
		{
			UUID sceneHandle = Project::GetActive()->GetConfig().SceneHandle;
			if (!sceneHandle || m_File.Index.Scenes.at(sceneHandle).Assets.find(handle) == m_File.Index.Scenes.at(sceneHandle).Assets.end())
				return false;

			m_File.Index.Scenes.at(sceneHandle).Assets.erase(handle);
		}
		return true;
	}

}
