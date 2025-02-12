#include "GE/GEpch.h"

#include "Project.h"
#include "Serializer/ProjectSerializer.h"

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
		if (s_ActiveProject->m_Config.RuntimeScene && s_ActiveProject->m_Config.RuntimeScene->IsStopped())
		{
			ProjectSerializer serializer(s_ActiveProject);
			if (serializer.Serialize(path))
			{
				s_ActiveProject->m_AssetManager->SerializeAssets();
				GE_CORE_INFO("Project Serialization Complete");

				// Update for Scene Asset
				s_ActiveProject->m_AssetManager->DeserializeAssets();
				return true;
			}
		}
		GE_CORE_ERROR("Project Serialization Failed");
		return false;
	}

	void Project::SetSceneHandle(UUID handle)
	{
		if (s_ActiveProject->m_Config.RuntimeScene && !s_ActiveProject->m_Config.RuntimeScene->IsStopped())
			s_ActiveProject->SceneStop();

		s_ActiveProject->m_Config.RuntimeScene = GetAssetCopy<Scene>(handle);
		if (s_ActiveProject->m_Config.RuntimeScene)
			s_ActiveProject->m_Config.SceneHandle = handle;
	}

	void Project::SceneStart(const Scene::State& state)
	{
		if (m_Config.RuntimeScene)
			m_Config.RuntimeScene->OnStart(state, Application::GetFramebuffer()->GetWidth(), Application::GetFramebuffer()->GetHeight());
	}

	void Project::SceneUpdate(Timestep ts)
	{
		if (m_Config.RuntimeScene)
			m_Config.RuntimeScene->OnUpdate(ts);
	}

	void Project::SceneStop()
	{
		if (m_Config.RuntimeScene)
			m_Config.RuntimeScene->OnStop();
	}

	void Project::SceneStep(int steps)
	{
		if (m_Config.RuntimeScene)
			m_Config.RuntimeScene->OnStep(steps);
	}

	bool Project::SceneEvent(Event& e, Entity entity)
	{
		if (m_Config.RuntimeScene && m_Config.RuntimeScene->HasComponent<IDComponent>(entity))
		{
			return m_Config.RuntimeScene->OnEvent(e, entity);
		}

		return false;
	}

	Ref<Scene> Project::SceneReset()
	{
		if (Ref<Scene> newRuntimeScene = GetAssetCopy<Scene>(m_Config.SceneHandle))
		{
			if (m_Config.RuntimeScene && !m_Config.RuntimeScene->IsStopped())
				m_Config.RuntimeScene->OnStop();
			return m_Config.RuntimeScene = newRuntimeScene;
		}
		return nullptr;
	}

#pragma region Tag Control

	bool Project::EventScene(Event& e, Entity entity)
	{
		return s_ActiveProject->SceneEvent(e, entity);
	}

	const std::string& Project::GetStrByTag(uint32_t id)
	{
		if (s_ActiveProject->m_Config.AllTags.find(id) != s_ActiveProject->m_Config.AllTags.end())
		{
			return s_ActiveProject->m_Config.AllTags.at(id);
		}

		return s_ActiveProject->m_Config.AllTags[0] = std::string("Default");
	}

	const uint32_t Project::GetTagFromStr(const std::string& tag)
	{
		if (!tag.empty())
		{
			for (auto& [id, str] : s_ActiveProject->m_Config.AllTags)
			{
				if (str == tag)
					return id;
			}
		}
		return s_ActiveProject->m_Config.AllTags.begin()->first;
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