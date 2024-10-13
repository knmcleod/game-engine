#pragma once

#include "AssetInfo.h"
#include "../Assets/Asset.h"

#include <unordered_set>
#include <filesystem>

namespace GE
{
	class AssetPack
	{
		friend class RuntimeAssetManager;
	public:
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
			};

			/*
			* Contains
			* - Offset : Offset of IndexTable relative to SOF(start of file)
			* - Size : Size of all Scenes
			* - Scenes : map<UUID, SceneInfo>
			*/

			struct IndexTable
			{
				uint64_t Size = 0;	// Size of all Scenes, includes Scene.size()
				std::map<uint64_t, SceneInfo> Scenes = std::map<uint64_t, SceneInfo>();  // UUID, SceneInfo
			};

			/*
			* File Path to Asset Pack
			* Use extension .gap
			*/
			std::filesystem::path Path;
			Header FileHeader;
			IndexTable Index;
		};

		AssetPack(const std::filesystem::path& filePath = "assetPack.gap");
		~AssetPack();

		void ClearAllFileData();

		/*
		* Returns true if given handle is found in all Handles across Scenes
		*/
		bool HandleExists(UUID handle);

		/*
		* Returns AssetInfo given handle using Project Scene handle
		* if given handle is found, set AssetInfo
		* if not found, search all Scenes in AssetPack::File & set AssetInfo
		* else, return empty AssetInfo
		*/
		const AssetInfo& GetAssetInfo(UUID handle);

		/*
		* Adds asset by Info to Pack File for current Project Scene
		* if asset is not Scene, Scene must already be in Pack File
		* else, Scene added to all Scenes
		*/
		bool AddAsset(Ref<Asset> asset, const AssetInfo& assetInfo);
		/*
		* Removes asset from Pack File using handle for current Project Scene
		* if asset is not Scene, Scene must already be in pack
		* else, Scene is removed from all Scenes
		*/
		bool RemoveAsset(Ref<Asset> asset);

	private:
		File m_File;

		// All currently added asset & entity handles, including Scenes
		std::unordered_set<UUID> m_HandleIndex;

		AssetInfo m_EmptyAssetInfo = AssetInfo();
	};
}

