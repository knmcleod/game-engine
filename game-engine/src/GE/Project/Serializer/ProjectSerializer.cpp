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

		const auto& spec = Project::GetActive()->GetSpec();
		GE_CORE_TRACE("Serializing Project: '{0}'", spec.Name);

		YAML::Emitter out;
		out << YAML::BeginMap; //
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap; // Project

			out << YAML::Key << "Name" << YAML::Value << spec.Name;
			out << YAML::Key << "ProjectPath" << YAML::Value << spec.ProjectPath.string();
			out << YAML::Key << "AssetPath" << YAML::Value << spec.AssetPath.string();
			out << YAML::Key << "SceneHandle" << YAML::Value << spec.SceneHandle;
			out << YAML::Key << "ScriptPath" << YAML::Value << spec.ScriptPath.string();

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
			GE_CORE_WARN("Cannot Deserialize Project. File path is empty.");
			return false;
		}
		
		auto& spec = m_Project->GetSpec();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filePath.string());
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load project file '{0}'\n     {1}", filePath, e.what());
			return false;
		}

		YAML::Node projectData = data["Project"];

		spec.Name = projectData["Name"].as<std::string>();
		GE_CORE_TRACE("Deserializing Project: '{0}'", spec.Name);

		//projects/demo
		spec.ProjectPath = projectData["ProjectPath"].as<std::string>(); // equivalent to filePath.parent_path()
		//assets
		spec.AssetPath = projectData["AssetPath"].as<std::string>();
		//
		spec.SceneHandle = projectData["SceneHandle"].as<uint64_t>();
		//scripts/Resources/Binaries/demo.dll
		spec.ScriptPath = projectData["ScriptPath"].as<std::string>();

		return true;
	}
}