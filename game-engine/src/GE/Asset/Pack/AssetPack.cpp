#include "GE/GEpch.h"

#include "AssetPack.h"

#include "../Serializer/AssetSerializer.h"

namespace GE
{
    template<typename T>
    Ref<T> AssetPack::GetAsset(UUID handle)
    {
        const AssetPackFile::AssetInfo* assetInfo = nullptr;

        bool found = false;
		UUID sceneHandle = Project::GetActive()->GetSpec().SceneHandle;

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


		Ref<T> asset = AssetSerializer::ImportAsset(handle, assetInfo);
        if (!asset)
            return nullptr;

        return asset;
    }

    uint64_t AssetPack::GetBuildVersion()
    {
        return m_File.Header.BuildVersion;
    }

    bool AssetPack::HandleExists(UUID handle)
    {
        return m_HandleIndex.find(handle) != m_HandleIndex.end();
    }

    bool AssetPack::AddAsset(Ref<Asset> asset)
    {
        UUID sceneHandle = Project::GetActive()->GetSpec().SceneHandle;
		if (!sceneHandle)
			return false;

		AssetPackFile::AssetInfo assetInfo;
		assetInfo.Type = (uint16_t)asset->GetType();
        m_File.Index.Scenes.at(sceneHandle).Assets.emplace(asset->GetHandle(), assetInfo);
		return true;
    }

	bool AssetPack::RemoveAsset(Ref<Asset> asset)
	{
		UUID handle = asset->GetHandle();
		if (asset->GetType() == AssetType::Scene && m_File.Index.Scenes.find(handle) != m_File.Index.Scenes.end())
		{
			m_File.Index.Scenes.erase(handle);
			return true;
		}
		else
		{
			UUID sceneHandle = Project::GetActive()->GetSpec().SceneHandle;
			if (!sceneHandle || m_File.Index.Scenes.at(sceneHandle).Assets.find(handle) == m_File.Index.Scenes.at(sceneHandle).Assets.end())
				return false;

			m_File.Index.Scenes.at(sceneHandle).Assets.erase(handle);
		}
		return true;
	}

    bool AssetPack::Serialize(const std::filesystem::path& filePath)
    {
		// gap file

		// header, info [] = byte
		//  [4] signature "GAP"
		//  [4] Version // File Format Version
		//  [8] Build Version // Time Built
		
		// index, data
		//  [8] App Binary Offset // Start of
		//  [8] App Binary Size   // Size of
		//  [?] Scene Map
		//      [8] Asset Handle    : Key
		//      SceneInfo           : Value
		//          [8] Packed Offset
		//          [8] Packed Size
		//          [2] Flags
		//          [?] Asset Map
		//              [8] Asset Handle    : Key
		//              AssetInfo           : Value
		//                  [8] Packed Offset
		//                  [8] Packed Size
		//                  [2] Flags
		//                  [2] Asset Type

		std::ofstream stream(filePath, std::ios::out | std::ios::binary | std::ios::app);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Binary file to write.");
			return false;
		}

		// Header
		stream.write(m_File.Header.HEADER, sizeof(m_File.Header.HEADER));
		stream.write(reinterpret_cast<const char*>(&m_File.Header.Version),
			sizeof(m_File.Header.Version));
		stream.write(reinterpret_cast<const char*>(&m_File.Header.BuildVersion),
			sizeof(m_File.Header.BuildVersion));

		// Index
		stream.write(reinterpret_cast<const char*>(&m_File.Index.Offset), sizeof(m_File.Index.Offset));
		stream.write(reinterpret_cast<const char*>(&m_File.Index.Size), sizeof(m_File.Index.Size));
		stream.write(reinterpret_cast<const char*>(m_File.Index.Scenes.size()), sizeof(m_File.Index.Scenes.size()));
		for (const auto& [sceneHandle, sceneInfo] : m_File.Index.Scenes)
		{
			stream.write(reinterpret_cast<const char*>(sceneHandle), sizeof(sceneHandle));
			stream.write(reinterpret_cast<const char*>(&sceneInfo.Offset), sizeof(sceneInfo.Offset));
			stream.write(reinterpret_cast<const char*>(&sceneInfo.Size), sizeof(sceneInfo.Size));
			stream.write(reinterpret_cast<const char*>(sceneInfo.Flags), sizeof(sceneInfo.Flags));
			stream.write(reinterpret_cast<const char*>(sceneInfo.Assets.size()), sizeof(sceneInfo.Assets.size()));
			for (const auto& [assetHandle, assetInfo] : sceneInfo.Assets)
			{
				stream.write(reinterpret_cast<const char*>((uint64_t)assetHandle), sizeof((uint64_t)assetHandle));
				stream.write(reinterpret_cast<const char*>(assetInfo.Offset), sizeof(assetInfo.Offset));
				stream.write(reinterpret_cast<const char*>(assetInfo.Size), sizeof(assetInfo.Size));
				stream.write(reinterpret_cast<const char*>(assetInfo.Flags), sizeof(assetInfo.Flags));
				stream.write(reinterpret_cast<const char*>(assetInfo.Type), sizeof(assetInfo.Type));

			}

		}
		stream.close();

		bool status = stream.good();
		if (status)
		{
			const auto& index = m_File.Index;
			for (const auto& [sceneHandle, sceneInfo] : index.Scenes)
			{
				m_HandleIndex.insert(sceneHandle);
			}
		}

		return status;
    }

    bool AssetPack::Deserialize(const std::filesystem::path& filePath)
    {
		std::ifstream stream(filePath, std::ios::out | std::ios::binary);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Binary file to read.");
			return false;
		}

		// Header
		stream.read((char*)&m_File.Header.HEADER, sizeof(m_File.Header.HEADER));
		stream.read((char*)&m_File.Header.Version, sizeof(m_File.Header.Version));
		stream.read((char*)&m_File.Header.BuildVersion, sizeof(m_File.Header.BuildVersion));

		// Index
		stream.read((char*)&m_File.Index.Offset, sizeof(m_File.Index.Offset));
		stream.read((char*)&m_File.Index.Size, sizeof(m_File.Index.Size));
		int sceneCount = 0;
		stream.read((char*)&sceneCount, sizeof(sceneCount));
		for (int i = 0; i < sceneCount; i++)
		{
			uint64_t sceneHandle = 0;
			stream.read((char*)&sceneHandle, sizeof(sceneHandle));
			AssetPackFile::SceneInfo sceneInfo;
			stream.read((char*)&sceneInfo.Offset, sizeof(sceneInfo.Offset));
			stream.read((char*)&sceneInfo.Size, sizeof(sceneInfo.Size));
			stream.read((char*)&sceneInfo.Flags, sizeof(sceneInfo.Flags));

			m_File.Index.Scenes[sceneHandle] = sceneInfo;

			int assetCount = 0;
			stream.read((char*)&assetCount, sizeof(assetCount));
			for (int j = 0; j < assetCount; j++)
			{
				uint64_t assetHandle = 0;
				stream.read((char*)&assetHandle, sizeof(assetHandle));
				AssetPackFile::AssetInfo assetInfo;
				stream.read((char*)&assetInfo.Offset, sizeof(assetInfo.Offset));
				stream.read((char*)&assetInfo.Size, sizeof(assetInfo.Size));
				stream.read((char*)&assetInfo.Flags, sizeof(assetInfo.Flags));
				stream.read((char*)&assetInfo.Type, sizeof(assetInfo.Type));

				m_File.Index.Scenes[sceneHandle].Assets[assetHandle] = assetInfo;

			}
		}

		stream.close();
		return stream.good();
    }

}
