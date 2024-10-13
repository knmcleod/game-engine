#include "GE/GEpch.h"

#include "Project.h"
#include "Serializer/ProjectSerializer.h"

#include "GE/Asset/Assets/Scene/Scene.h"
#include "GE/Asset/RuntimeAssetManager.h"

#include "GE/Core/Application/Application.h"

namespace GE
{
	Ref<Project> Project::s_ActiveProject = nullptr;

	void Project::Init()
	{
		s_ActiveProject = CreateRef<Project>();
	}

	void Project::Shutdown()
	{
		s_ActiveProject->m_Config.AllTags.clear();
		s_ActiveProject->m_Config.AllTags = std::map<uint32_t, std::string>();
	}

	bool Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();
		GE_CORE_INFO("Project Deserialization Start");
		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			s_ActiveProject = project;
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

			// Update for Scene Asset
			s_ActiveProject->m_AssetManager->DeserializeAssets();
			return true;
		}
		GE_CORE_ERROR("Project Serialization Failed");
		return false;
	}

	void Project::SetSceneHandle(UUID handle)
	{
		if (s_ActiveProject->m_Config.RuntimeScene && handle == s_ActiveProject->m_Config.RuntimeScene->GetHandle())
			return;

		if (s_ActiveProject->m_Config.RuntimeScene && !s_ActiveProject->m_Config.RuntimeScene->IsStopped())
			s_ActiveProject->m_Config.RuntimeScene->OnStop();

		s_ActiveProject->m_Config.RuntimeScene = GetAssetCopy<Scene>(handle);
		if (s_ActiveProject->m_Config.RuntimeScene)
			s_ActiveProject->m_Config.SceneHandle = handle;
	}

	void Project::StartScene(const Scene::State& state)
	{
		if (Ref<Scene> runtimeScene = GetRuntimeScene())
			runtimeScene->OnStart(state, Application::GetFramebuffer()->GetWidth(), Application::GetFramebuffer()->GetHeight());
	}

	void Project::UpdateScene(Timestep ts)
	{
		if (Ref<Scene> runtimeScene = GetRuntimeScene())
			runtimeScene->OnUpdate(ts);
	}

	void Project::StopScene()
	{
		if (Ref<Scene> runtimeScene = GetRuntimeScene())
			runtimeScene->OnStop();
	}

	void Project::StepScene(int steps)
	{
		if (Ref<Scene> runtimeScene = GetRuntimeScene())
			runtimeScene->OnStep(steps);
	}

	bool Project::SceneEvent(Event& e, Entity entity)
	{
		if (Ref<Scene> runtimeScene = GetRuntimeScene())
		{
			if (entity && entity.HasComponent<IDComponent>())
			{
				return runtimeScene->OnEvent(e, entity);
			}
		}

		return false;
	}

	bool Project::ResetScene()
	{
		if (Ref<Scene> newRuntimeScene = GetAssetCopy<Scene>(s_ActiveProject->m_Config.SceneHandle))
		{
			if (s_ActiveProject->m_Config.RuntimeScene && !s_ActiveProject->m_Config.RuntimeScene->IsStopped())
				s_ActiveProject->m_Config.RuntimeScene->OnStop();
			s_ActiveProject->m_Config.RuntimeScene = newRuntimeScene;
			return true;
		}
		return false;
	}

#pragma region Tag Control

	const std::string& Project::GetTagByKey(uint32_t id)
	{
		if (s_ActiveProject->m_Config.AllTags.find(id) != s_ActiveProject->m_Config.AllTags.end())
		{
			return s_ActiveProject->m_Config.AllTags.at(id);
		}

		return s_ActiveProject->m_Config.AllTags[0] = std::string("Default");
	}

	uint32_t Project::GetIDFromTag(const std::string& tag)
	{
		if (!tag.empty())
		{
			for (auto& [id, str] : s_ActiveProject->m_Config.AllTags)
			{
				if (str == tag)
					return id;
			}
		}
		return 0;
	}

	bool Project::TagExists(const std::string& tag)
	{
		for (const auto& [tagID, tagStr] : s_ActiveProject->m_Config.AllTags)
		{
			if (tagStr == tag)
				return true;
		}
		return false;
	}

	bool Project::TagIDExists(uint32_t id)
	{
		return s_ActiveProject->m_Config.AllTags.find(id) != s_ActiveProject->m_Config.AllTags.end();
	}

	bool Project::AddTag(const std::string& tag, uint32_t id)
	{
		if (TagIDExists(id) || TagExists(tag))
		{
			GE_CORE_WARN("Could not add tag. Tag already exists.");
			return false;
		}
		s_ActiveProject->m_Config.AllTags.emplace(id, tag);
		return true;
	}

	bool Project::RemoveTag(uint32_t id)
	{
		if (TagIDExists(id))
		{
			s_ActiveProject->m_Config.AllTags.erase(id);
			return true;
		}
		return false;
	}

#pragma endregion

}