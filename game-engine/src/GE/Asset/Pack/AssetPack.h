#pragma once

#include "AssetInfo.h"
#include "GE/Asset/Assets/Asset.h"

#include <unordered_set>
#include <filesystem>

namespace GE
{
	class AssetPack
	{
		friend class AssetSerializer;
		friend class RuntimeAssetManager;
	public:
		AssetPack(const std::filesystem::path& filePath = "assetPack.gap");
		~AssetPack();

		void ClearAllFileData()
		{
			for (auto& [uuid, sceneInfo] : m_File.Index.Scenes)
			{
				delete[] sceneInfo.p_Data;
				sceneInfo.p_Data = nullptr;

				for (auto& [uuid, assetInfo] : sceneInfo.m_Assets)
				{
					delete[] assetInfo.p_Data;
					assetInfo.p_Data = nullptr;
				}

				for (auto& [uuid, entityInfo] : sceneInfo.m_Entities)
				{
					delete[] entityInfo.Data;
					entityInfo.Data = nullptr;
				}
			}
		}

		template<typename T>
		T* GetAssetInfo(const AssetInfo& assetInfo)
		{
			if (!AssetInfoExists(assetInfo))
				return nullptr;

			AssetInfo* ai;
			for (auto& [uuid, sceneInfo] : m_File.Index.Scenes)
			{
				if (assetInfo == (AssetInfo&)sceneInfo)
					ai = &sceneInfo;

				// Check child Assets if Scene is not Asset
				for (auto& [uuid, childAssetInfo] : m_File.Index.Scenes.at(uuid).m_Assets)
				{
					if (assetInfo == childAssetInfo)
						ai = &childAssetInfo;

				}
			}
			return static_cast<T*>(ai);
		}

		bool AssetInfoExists(const AssetInfo& assetInfo)
		{
			for (auto& [uuid, sceneInfo] : m_File.Index.Scenes)
			{
				if (assetInfo == (AssetInfo&)sceneInfo)
					return true;

				// Check child Assets if Scene is not Asset
				for (auto& [uuid, childAssetInfo] : m_File.Index.Scenes.at(uuid).m_Assets)
				{
					if (assetInfo == childAssetInfo)
						return true;

				}
			}
			GE_CORE_WARN("AssetInfo does not exist in Pack.");
			return false;
		}

		/*
		* Returns Pack File Header Build Version
		* Build Version Format = Data/Time
		*/
		const uint64_t& AssetPack::GetBuildVersion() const
		{
			return m_File.FileHeader.BuildVersion;
		}

		/*
		* Returns sizeof entire File::Header
		*/
		uint64_t GetHeaderSize()
		{
			return sizeof(m_File.FileHeader.HEADER)
				+ sizeof(m_File.FileHeader.Version)
				+ sizeof(m_File.FileHeader.BuildVersion);
		}



		/*
		* Returns true if given handle is found in all Handles across Scenes
		*/
		bool HandleExists(UUID handle);

		/*
		* Returns asset of type T in current Project Scene
		* if Scene found in AssetPack::File, search for given handle in SceneInfo::Assets
		* if given handle is found, set AssetInfo
		* if not found, search all Scenes in AssetPack::File & set AssetInfo
		* else, return nullptr
		* DeserializeAsset(AssetInfo) and return
		*/
		template<typename T>
		Ref<T> GetAsset(UUID handle);

		/*
		* Adds asset by Info to Pack File for current Project Scene
		* if asset is not Scene, Scene must already be in Pack File
		* else, Scene added to all Scenes
		*/
		bool AddAsset(Ref<Asset> asset, const AssetInfo* assetInfo = 0);
		/*
		* Removes asset from Pack File using handle for current Project Scene
		* if asset is not Scene, Scene must already be in pack
		* else, Scene is removed from all Scenes
		*/
		bool RemoveAsset(Ref<Asset> asset);

		struct File
		{
			/*
			* Contains
			* - Header : file extension
			* - Version : file format
			* - Build Version : Data/Time format
			*/
			struct Header
			{
				const char HEADER[3] = { 'G', 'A', 'P' };
				uint32_t Version = 1;
				uint64_t BuildVersion = 0; // Data/Time format
			};

			/*
			* Contains
			* - Offset : Offset of IndexTable relative to SOF(start of file)
			* - Size : Size of all Scenes
			* - Scenes : map<UUID, SceneInfo>
			*/

			struct IndexTable
			{
				uint64_t Offset = 0;// Offset of IndexTable relative to SOF(start of file)
				uint64_t Size = 0;	// Size of all Scenes, includes Scene.size()
				std::map<uint64_t, SceneInfo> Scenes;  // UUID, SceneInfo
			};

			/* 
			* File Path to Asset Pack
			* Use extension .gap
			*/
			std::filesystem::path Path;
			Header FileHeader;
			IndexTable Index;
		};

	private:
		File m_File;

		// All currently added asset & entity handles, including Scenes
		std::unordered_set<UUID> m_HandleIndex;

	};
}

