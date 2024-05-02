#pragma once

#include "GE/Asset/Asset.h"

#include <unordered_set>
#include <filesystem>

namespace GE
{
	class AssetPack
	{
		friend class AssetSerializer;
		friend class RuntimeAssetManager;
	public:
		template<typename T>
		Ref<T> GetAsset(UUID handle);

		uint64_t GetBuildVersion() const;
		bool HandleExists(UUID handle);

		bool AddAsset(Ref<Asset> asset);
		bool RemoveAsset(Ref<Asset> asset);

		struct File
		{
			struct AssetInfo
			{
				uint64_t Offset;// Packed Data Offset
				uint64_t Size;	// Packed Data Size
				uint16_t Type;	// Asset Type: Texture2D, etc.
				uint16_t Flags = 0; // compressed type, etc.
			};

			struct SceneInfo
			{

				uint64_t Offset;// Packed Data Offset
				uint64_t Size;	// Packed Data Size
				uint16_t Flags = 0; // compressed type, etc.
				std::map<uint64_t, AssetInfo> Assets; // AssetPack->GetAssetInfo(Handle)
			};

			struct IndexTable
			{
				uint64_t Offset;// App Binary Offset
				uint64_t Size;	// App Binary Size
				std::map<uint64_t, SceneInfo> Scenes;  // AssetPack->GetSceneInfo(Handle)
			};

			struct FileHeader
			{
				const char HEADER[3] = { 'G', 'A', 'P' };
				uint32_t Version = 1;
				uint64_t BuildVersion = 0; // Data/Time format
			};

			std::filesystem::path Path;
			FileHeader Header;
			IndexTable Index;
		};

	private:
		File m_File;

		std::unordered_set<UUID> m_HandleIndex;

	};
}

