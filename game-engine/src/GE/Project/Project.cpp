#include "GE/GEpch.h"

#include "Project.h"
#include "Serializer/ProjectSerializer.h"

#include "GE/Asset/RuntimeAssetManager.h"

#include "GE/Scripting/Scripting.h"

namespace GE
{
	Ref<Project> Project::s_ActiveProject = nullptr;

	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}

	void Project::Shutdown()
	{
		Scripting::Shutdown();
	}

	bool Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();
		GE_CORE_INFO("Project Deserialization Start");
		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			s_ActiveProject = project;

			Scripting::Init();
			GE_CORE_INFO("Project Deserialization Complete");
			return true;
		}
		GE_CORE_ERROR("Project Deserialization Failed");
		return false;
	}

	bool Project::Save(const std::filesystem::path& path)
	{
		GE_CORE_INFO("Project Serialization Start");
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			s_ActiveProject->m_AssetManager->SerializeAssets();
			GE_CORE_INFO("Project Serialization Complete");

			return true;
		}
		GE_CORE_ERROR("Project Serialization Failed");
		return false;
	}

}