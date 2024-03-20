#include "GE/GEpch.h"

#include "Project.h"
#include "Serializer/ProjectSerializer.h"

namespace GE
{
	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();

		GE_CORE_TRACE("Deserializing Project at Path = {}", path.string());
		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			s_ActiveProject = project;
			GE_CORE_INFO("Project Deserialization Complete");
			return s_ActiveProject;
		}
		return nullptr;
	}

	bool Project::Save(const std::filesystem::path& path)
	{
		GE_CORE_TRACE("Serializing Project at Path = {}", path.string());
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			GE_CORE_INFO("Project Serialization Complete");
			return true;
		}
		return false;
	}
}