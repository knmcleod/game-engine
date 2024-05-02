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
			GE_CORE_WARN("Cannot Serialize Project. Project is undefined.");
			return false;
		}

		if (filePath.extension().string() != ".gproj")
		{
			GE_CORE_WARN("Could not save {0}\n\tFile extension is not .gproj.\n\tExtension: {1}", filePath.string(), filePath.extension().string());
			return false;
		}

		YAML::Emitter out;
		out << YAML::BeginMap; //
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap; // Project

			out << YAML::Key << "Name" << YAML::Value << m_Project->m_Config.Name;
			out << YAML::Key << "ProjectPath" << YAML::Value << m_Project->m_Config.ProjectPath.string();
			out << YAML::Key << "AssetPath" << YAML::Value << m_Project->m_Config.AssetPath.string();
			out << YAML::Key << "SceneHandle" << YAML::Value << m_Project->m_Config.SceneHandle;
			out << YAML::Key << "ScriptPath" << YAML::Value << m_Project->m_Config.ScriptPath.string();

			out << YAML::EndMap;
		}
		out << YAML::EndMap; //

		std::ofstream fout(filePath);
		fout << out.c_str();

		GE_CORE_TRACE("Serializing Project\n\tFilePath: {0}\n\tName: {1}", filePath.string(), m_Project->m_Config.Name);

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filePath)
	{
		if (filePath.empty())
		{
			GE_CORE_WARN("Cannot Deserialize Project. File path is empty.");
			return false;
		}

		if (filePath.extension().string() != ".gproj")
		{
			GE_CORE_WARN("Could not load {0}\n\tFile extension is not .gproj.\n\tExtension: {1}", filePath.string(), filePath.extension().string());
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

		m_Project->m_Config.Name = projectData["Name"].as<std::string>();
		//projects/demo
		m_Project->m_Config.ProjectPath = projectData["ProjectPath"].as<std::string>(); // equivalent to filePath.parent_path()
		//assets
		m_Project->m_Config.AssetPath = projectData["AssetPath"].as<std::string>();
		//UUID. See AssetManager
		m_Project->m_Config.SceneHandle = projectData["SceneHandle"].as<uint64_t>();
		//scripts/Resources/Binaries/demo.dll
		m_Project->m_Config.ScriptPath = projectData["ScriptPath"].as<std::string>();
		GE_CORE_TRACE("Deserializing Project\n\tFilePath: {0}\n\tName: {1}", filePath.string(), m_Project->m_Config.Name);
		return true;
	}
}