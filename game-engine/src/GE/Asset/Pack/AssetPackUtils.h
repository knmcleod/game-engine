#pragma once

namespace GE
{
	struct AssetPackFile
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

		FileHeader Header;
		IndexTable Index;
	};
}
