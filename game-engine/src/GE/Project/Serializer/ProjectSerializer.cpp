#include "GE/GEpch.h"

#include "ProjectSerializer.h"

#include <yaml-cpp/yaml.h>

namespace GE
{
	ProjectSerializer::ProjectSerializer(Ref<Project> project) : m_Project(project)
	{
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filePath)
	{
		if (filePath.empty())
		{
			GE_CORE_WARN("Cannot Serialize Project.\n\tFile path is empty.");
			return false;
		}

		if (filePath.extension().string() != ".gproj")
		{
			GE_CORE_WARN("Could not save {0}\n\tFile extension is not .gproj.\n\tExtension: {1}", filePath.string(), filePath.extension().string());
			return false;
		}

		GE_CORE_TRACE("Serializing Project\n\tFilePath : {0}\n\tName : {1}", filePath.string(), m_Project->m_Config.Name);

		YAML::Emitter out;
		out << YAML::BeginMap; //
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap; // Project

			out << YAML::Key << "Name" << YAML::Value << m_Project->m_Config.Name;
			out << YAML::Key << "Width" << YAML::Value << m_Project->m_Config.Width;
			out << YAML::Key << "Height" << YAML::Value << m_Project->m_Config.Height;
			out << YAML::Key << "SceneHandle" << YAML::Value << m_Project->m_Config.SceneHandle;
			out << YAML::Key << "ProjectPath" << YAML::Value << m_Project->m_Config.ProjectPath.string();
			out << YAML::Key << "AssetPath" << YAML::Value << m_Project->m_Config.AssetPath.string();
			out << YAML::Key << "ScriptAssetPath" << YAML::Value << m_Project->m_Config.ScriptAssetPath.string();
			out << YAML::Key << "ScriptCorePath" << YAML::Value << m_Project->m_Config.ScriptCorePath.string();
			out << YAML::Key << "ScriptAppPath" << YAML::Value << m_Project->m_Config.ScriptAppPath.string();

			out << YAML::EndMap;
		}
		out << YAML::EndMap; //

		std::ofstream fout(filePath);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filePath)
	{
		if (filePath.empty())
		{
			GE_CORE_WARN("Cannot Deserialize Project.\n\tFile path is empty.");
			return false;
		}

		if (filePath.extension().string() != ".gproj")
		{
			GE_CORE_WARN("Could not load {0}\n\tFile extension is not .gproj.\n\tExtension: {1}", filePath.string().c_str(), filePath.extension().string().c_str());
			return false;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filePath.string());
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("\n\tFailed to load project file. {0}\n\t{1}", filePath.string(), e.what());
			return false;
		}

		YAML::Node projectData = data["Project"];
		if (!projectData)
			return false;

		if(auto& name = projectData["Name"])
			m_Project->m_Config.Name = name.as<std::string>();
		GE_CORE_TRACE("Deserializing Project\n\tFilePath: {0}\n\tName: {1}", 
			filePath.string(), m_Project->m_Config.Name);

		if(auto& width = projectData["Width"])
			m_Project->m_Config.Width = width.as<uint32_t>();
		if (auto& height = projectData["Height"])
			m_Project->m_Config.Height = height.as<uint32_t>();

		if(auto& sceneHandle = projectData["SceneHandle"]) // UUID. See AssetManager
			m_Project->m_Config.SceneHandle = sceneHandle.as<uint64_t>();

		if (auto& projectPath = projectData["ProjectPath"]) // Example = projects/demo
			m_Project->m_Config.ProjectPath = projectPath.as<std::string>(); // equivalent to filePath.parent_path()
		if (auto& assetPath = projectData["AssetPath"]) // Example = assets
			m_Project->m_Config.AssetPath = assetPath.as<std::string>();
		if (auto& scriptAssetPath = projectData["ScriptAssetPath"]) // Example = scripts/src, relative to AssetPath
			m_Project->m_Config.ScriptAssetPath = scriptAssetPath.as<std::string>();

		if(auto& scriptCorePath = projectData["ScriptCorePath"]) // Example = scripts/bin/GE-ScriptCore.dll
			m_Project->m_Config.ScriptCorePath = scriptCorePath.as<std::string>();
		if (auto& scriptAppPath = projectData["ScriptAppPath"]) // Example = scripts/bin/projName.dll
			m_Project->m_Config.ScriptAppPath = scriptAppPath.as<std::string>();

		return true;
	}
}