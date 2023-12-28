#pragma once
#include "GE/Core/Time/Time.h"
#include "GE/Rendering/Camera/Editor/EditorCamera.h"
#include "Components/Components.h"

#include <entt/entt.hpp>

class b2World;

namespace GE
{
	class Scene
	{
		friend class SceneSerializer;
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class EditorLayer;
	private:
		b2World* m_PhysicsWorld = nullptr;

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		enum class SceneState
		{
			Play = 0,
			Simulate = 1,
			Stop = 2
		};
		SceneState m_SceneState = SceneState::Stop;

		template<typename T>
		void OnComponentAdded(Entity entity);

	public:
		Scene();
		~Scene();
		
		static Ref<Scene> Scene::Copy(const Ref<Scene> scene);

		Entity GetPrimaryCameraEntity();
		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		void ResizeViewport(uint32_t width, uint32_t height);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnEditorUpdate(Timestep timestep, EditorCamera& camera);
		void OnRuntimeUpdate(Timestep timestep);
	};
}