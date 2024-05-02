#include "GE/GEpch.h"

#include "Project.h"
#include "Serializer/ProjectSerializer.h"

namespace GE
{
	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();
		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			s_ActiveProject = project;
			GE_CORE_INFO("Project Deserialization Complete");
		}
		return s_ActiveProject;
	}

	bool Project::Save(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			GE_CORE_INFO("Project Serialization Complete");
			return true;
		}
		return false;
	}
}