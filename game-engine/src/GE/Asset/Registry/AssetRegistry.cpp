#include "GE/GEpch.h"

#include "AssetRegistry.h"

#include <yaml-cpp/yaml.h>

namespace GE
{
	AssetRegistry::AssetRegistry()
	{
		m_AssetRegistry = std::map<UUID, AssetMetadata>();
	}

	AssetMetadata& AssetRegistry::GetAssetMetadata(UUID handle)
	{
		return m_AssetRegistry.find(handle)->second;
	}

	bool AssetRegistry::AssetExists(UUID handle)
	{
		if (m_AssetRegistry.find(handle) == m_AssetRegistry.end())
			GE_CORE_WARN("Asset Handle does not exist in Registry.");

		return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool AssetRegistry::AddAsset(const AssetMetadata& metadata)
	{
		if (m_AssetRegistry.find(metadata.Handle) != m_AssetRegistry.end())
		{
			GE_CORE_TRACE("Asset already exists in Registry.");
			return false;
		}
		m_AssetRegistry.emplace(metadata.Handle, metadata);
		return true;
	}

	bool AssetRegistry::RemoveAsset(UUID handle)
	{
		if (!AssetExists(handle))
		{
			return false;
		}
		m_AssetRegistry.erase(handle);
		return true;
	}

	bool AssetRegistry::Serialize(const std::filesystem::path& filePath)
	{
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

		std::ofstream fout(filePath);
		fout << out.c_str();
		return true;
	}

	bool AssetRegistry::Deserialize(const std::filesystem::path& filePath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filePath.string());
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load asset registry file '{0}'\n     {1}", filePath, e.what());
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
			assetMetadata.Handle = handle;
			assetMetadata.FilePath = node["FilePath"].as<std::string>();
			assetMetadata.Type = AssetUtils::AssetTypeFromString(node["Type"].as<std::string>());

			m_AssetRegistry.emplace(handle, assetMetadata);
		}

		return true;
	}
}
