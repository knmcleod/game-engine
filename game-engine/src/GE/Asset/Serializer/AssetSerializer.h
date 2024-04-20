#pragma once

#include "../Asset.h"
#include "../Registry/AssetMetadata.h"
#include "../Pack/AssetPack.h"

#include "GE/Rendering/Textures/Texture.h"
#include "GE/Scene/Scene.h"

namespace GE
{
	using AssetImportFunction = std::function<Ref<Asset>(const AssetMetadata&)>;
	using AssetPackImportFunction = std::function<Ref<Asset>(const AssetPackFile::AssetInfo&)>;

	using AssetExportFunction = std::function<bool(UUID, const std::filesystem::path&)>;

	class AssetSerializer
	{
	public:
		static Ref<Asset> ImportAsset(const AssetMetadata& metadata);
		static Ref<Asset> ImportAsset(const AssetPackFile::AssetInfo& assetInfo);

		static bool ExportAsset(UUID handle, const AssetMetadata& metadata);
		static bool ExportAsset(UUID handle, const AssetPackFile::AssetInfo& assetInfo);
	private:
		// TODO: Implement Audio Sources & Listeners in Asset System

		static Ref<Asset> ImportScene(const AssetMetadata& metadata);
		static Ref<Asset> ImportSceneFromPack(const AssetPackFile::AssetInfo& assetInfo);
		
		static Ref<Asset> ImportTexture2D(const AssetMetadata& metadata);
		static Ref<Asset> ImportTexture2DFromPack(const AssetPackFile::AssetInfo& assetInfo);

		static bool ExportScene(UUID uuid, const std::filesystem::path& filePath);

		static std::map<AssetType, AssetImportFunction> s_AssetImportFuncs;
		static std::map<AssetType, AssetPackImportFunction> s_AssetPackImportFuncs;

		static std::map<AssetType, AssetExportFunction> s_AssetExportFuncs;
	};
}
