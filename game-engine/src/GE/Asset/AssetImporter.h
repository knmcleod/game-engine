#pragma once

#include "GE/Asset/Asset.h"
#include "GE/Asset/AssetMetadata.h"

#include "GE/Rendering/Textures/Texture.h"
#include "GE/Scene/Scene.h"

namespace GE
{
	using AssetImportFunction = std::function<Ref<Asset>(const AssetMetadata&)>;
	using AssetExportFunction = std::function<bool(UUID, const std::filesystem::path&)>;

	class AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(const AssetMetadata& metadata);
		static bool ExportAsset(UUID handle, const AssetMetadata& metadata);
	private:
		static Ref<Scene> ImportScene(const AssetMetadata& metadata);
		static Ref<Texture2D> ImportTexture2D(const AssetMetadata& metadata);

		static bool ExportScene(UUID uuid, const std::filesystem::path& filePath);

		static std::map<AssetType, AssetImportFunction> s_AssetImportFuncs;
		static std::map<AssetType, AssetExportFunction> s_AssetExportFuncs;
	};
}
