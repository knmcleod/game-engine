#include <GE/GE.h>
#include "EditorAssetManager.h"

#include "GE/Asset/AssetImporter.h"

#include <yaml-cpp/include/yaml-cpp/yaml.h>

namespace GE
{
	EditorAssetManager::~EditorAssetManager()
	{
		m_LoadedAssets.clear();
		m_AssetRegistry.clear();
	}

	/*
	* Returns nullptr if handle does NOT exist
	* or if asset can not be loaded
	*/
	Ref<Asset> EditorAssetManager::GetAsset(UUID handle)
	{
		if (!HandleExists(handle))
		{
			GE_CORE_WARN("Asset Handle does not exist.");
			return nullptr;
		}

		Ref<Asset> asset;
		if (!AssetLoaded(handle))
		{
			const AssetMetadata& metadata = GetMetadata(handle);
			asset = AssetImporter::ImportAsset(metadata);
			if (asset)
			{
				asset->SetHandle(handle);
				m_LoadedAssets[handle] = asset;
			}
		}
		else
			asset = m_LoadedAssets.at(handle);

		return asset;
	}

	Ref<Asset> EditorAssetManager::GetAsset(const AssetMetadata& metadata)
	{
		UUID handle = GetHandle(metadata);

		return GetAsset(handle);
	}

	bool EditorAssetManager::HandleExists(UUID handle)
	{
		return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool EditorAssetManager::HandleExists(const AssetMetadata& metadata)
	{
		UUID handle = GetHandle(metadata);
		if (handle == 0)
			return false;
		return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	const UUID EditorAssetManager::GetHandle(const AssetMetadata& metadata) const
	{
		for (auto& asset : m_AssetRegistry)
		{
			if (asset.second.FilePath == metadata.FilePath && asset.second.Type == metadata.Type)
				return asset.first;
		}
		return 0;
	}

	bool EditorAssetManager::AssetLoaded(UUID handle)
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(UUID handle)
	{
		static AssetMetadata s_NullMetadata;
		if (!HandleExists(handle))
			return s_NullMetadata;
		return m_AssetRegistry.find(handle)->second;
	}

	bool EditorAssetManager::SaveAsset(UUID handle)
	{
		if (!HandleExists(handle))
			return false;
		SerializeAssets();
		return AssetImporter::ExportAsset(handle, GetMetadata(handle));
	}

	bool EditorAssetManager::RemoveAsset(UUID handle)
	{
		if (HandleExists(handle))
		{
			if (AssetLoaded(handle))
				m_LoadedAssets.erase(handle);

			m_AssetRegistry.erase(handle);
			return true;
		}
		return false;
	}

	bool EditorAssetManager::SerializeAssets()
	{
		std::filesystem::path path(Project::GetAssetPath() / "assets.txt");

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				std::string filepathStr = metadata.FilePath.generic_string();
				out << YAML::Key << "FilePath" << YAML::Value << filepathStr;
				const std::string typeString = AssetUtils::AssetTypeToString(metadata.Type);
				out << YAML::Key << "Type" << YAML::Value << typeString.c_str();
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(path);
		fout << out.c_str();
		return true;
	}

	bool EditorAssetManager::DeserializeAssets()
	{
		std::filesystem::path filePath = Project::GetPathToAsset("assets.txt");
		std::filesystem::path path(filePath);

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load asset registry file '{0}'\n     {1}", path, e.what());
		}

		YAML::Node assetRegistryData = data["AssetRegistry"];
		if (!assetRegistryData)
		{
			GE_WARN("Cannot deserialize asset registry.");
			return false;
		}

		for (const auto& node : assetRegistryData)
		{
			UUID handle = node["Handle"].as<uint64_t>();
			AssetMetadata assetMetadata;
			assetMetadata.FilePath = node["FilePath"].as<std::string>();
			assetMetadata.Type = AssetUtils::AssetTypeFromString(node["Type"].as<std::string>());
			
			m_AssetRegistry[handle] = assetMetadata;
		}

		return true;
	}

}
