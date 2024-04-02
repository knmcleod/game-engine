#include "GE/GEpch.h"

#include "AssetImporter.h"
#include "AssetUtils.h"

#include "GE/Project/Project.h"
#include "GE/Scene/Serializer/SceneSerializer.h"

#include <stb_image.h>

namespace GE
{
	std::map<AssetType, AssetImportFunction> AssetImporter::s_AssetImportFuncs =
	{
		{ AssetType::Scene, AssetImporter::ImportScene },
		{ AssetType::Texture2D, AssetImporter::ImportTexture2D }
	};

	std::map<AssetType, AssetExportFunction> AssetImporter::s_AssetExportFuncs =
	{
		{ AssetType::Scene, AssetImporter::ExportScene }
	};

	Ref<Asset> AssetImporter::ImportAsset(const AssetMetadata& metadata)
	{
		if (s_AssetImportFuncs.find(metadata.Type) == s_AssetImportFuncs.end())
		{
			GE_CORE_ERROR("Importer function not found for Type: " + AssetUtils::AssetTypeToString(metadata.Type));
			return nullptr;
		}
		return s_AssetImportFuncs.at(metadata.Type)(metadata);
	}

	bool AssetImporter::ExportAsset(UUID handle, const AssetMetadata& metadata)
	{
		if (s_AssetExportFuncs.find(metadata.Type) == s_AssetExportFuncs.end())
		{
			GE_CORE_ERROR("Exporter function not found for Type: " + AssetUtils::AssetTypeToString(metadata.Type));
			return false;
		}
		return s_AssetExportFuncs.at(metadata.Type)(handle, metadata.FilePath);
	}

	Ref<Scene> AssetImporter::ImportScene(const AssetMetadata& metadata)
	{
		std::filesystem::path path = Project::GetPathToAsset(metadata.FilePath);
		GE_CORE_TRACE("Deserializing Scene at Path = {}", path.string());
		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer(scene);
		if (serializer.Deserialize(path))
		{
			GE_CORE_INFO("Scene Deserialization Complete");
		}
		return scene;
	}

	Ref<Texture2D> AssetImporter::ImportTexture2D(const AssetMetadata& metadata)
	{
		int width = 0, height = 0, channels = 4;
		stbi_set_flip_vertically_on_load(1);
		Buffer data;
		{
			GE_PROFILE_SCOPE("stbi_load - AssetImporter::ImportTexture2D");
			std::string path = Project::GetPathToAsset(metadata.FilePath).string();
			data.Data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		GE_CORE_ASSERT(data, "Failed to load stb image!");
		data.Size = width * height * channels;  // Assumed 1 byte per channel

		TextureConfiguration config;
		config.Height = height;
		config.Width = width;
		if (channels == 3)
		{
			config.InternalFormat = ImageFormat::RBG8;
			config.Format = DataFormat::RGB;
		}
		else if (channels == 4)
		{
			config.InternalFormat = ImageFormat::RBGA8;
			config.Format = DataFormat::RGBA;
		}
		else
			GE_CORE_WARN("Unsupported Texture2D Channels.");

		Ref<Texture2D> texture = Texture2D::Create(config, data);
		data.Release();

		return texture;

	}
	
	bool AssetImporter::ExportScene(UUID uuid, const std::filesystem::path& filePath)
	{
		std::filesystem::path path = Project::GetPathToAsset(filePath);
		GE_CORE_TRACE("Serializing Scene at Path = {}", path.string());
		Ref<Scene> scene = Project::GetAsset<Scene>(uuid);
		SceneSerializer serializer(scene);
		if (serializer.Serialize(path))
		{
			GE_CORE_INFO("Scene Serialization Complete");
			return true;
		}
		return false;
	}
}