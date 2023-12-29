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
		template<typename T>
		void OnComponentAdded(Entity entity);

		enum class SceneState
		{
			Run = 0,
			Simulate = 1,
			Stop = 2
		};
		SceneState m_SceneState = SceneState::Stop;

		b2World* m_PhysicsWorld = nullptr;

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		void InitializePhysics2D();
		void UpdatePhysics2D(Timestep timestep);
		void DestroyPhysics2D();
	public:
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		Scene();
		~Scene();
		
		static Ref<Scene> Copy(const Ref<Scene> scene);
		void Render(const EditorCamera& camera);

		Entity GetPrimaryCameraEntity();
		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		void ResizeViewport(uint32_t width, uint32_t height);

		void OnStop();

		void OnRuntimeStart();
		void OnRuntimeUpdate(Timestep timestep);

		void OnSimulationStart();
		void OnSimulationUpdate(Timestep timestep, EditorCamera& camera);

		void OnEditorUpdate(Timestep timestep, EditorCamera& camera);

	};
}