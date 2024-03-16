#pragma once

#include "GE/Core/Time/Time.h"
#include "GE/Core/UUID/UUID.h"

#include "GE/Rendering/Camera/Editor/EditorCamera.h"

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
			Stop = 2,
			Pause = 3 // Called during Run or Simulate
		};
		SceneState m_SceneState = SceneState::Stop;

		int m_StepFrames = 0;
		bool m_UseEditorCameraPaused = false;

		b2World* m_PhysicsWorld = nullptr;

		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		std::string m_Name = "Scene";

		void InitializePhysics2D();
		void UpdatePhysics2D(Timestep timestep);
		void DestroyPhysics2D();

		void InitializeScripting();
		void UpdateScripting(Timestep timestep);
		void DestroyScripting();
	public:
		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		Scene();
		Scene(const std::string& name);
		~Scene();
		
		std::string GetName() { return m_Name; }
		void SetName(std::string name) { m_Name = name; }

		static std::string Scene::SceneStateToString(Scene::SceneState state);

		// Returns true if the scenes state is Run. Does not account for Simulation
		bool IsRunning() const { return m_SceneState == SceneState::Run; }
		bool IsPaused() const { return m_SceneState == SceneState::Pause; }

		static Ref<Scene> Copy(const Ref<Scene> scene);
		void Render(const EditorCamera& camera);

		void OnEntitySelected(Entity entity);

		Entity GetPrimaryCameraEntity();
		Entity GetEntityByUUID(UUID uuid);
		Entity GetEntityByTag(char* tag);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		void OnResizeViewport(uint32_t width, uint32_t height);

		void OnStop();

		void OnRuntimeStart();
		void OnRuntimeUpdate(Timestep timestep);

		void OnSimulationStart();
		void OnSimulationUpdate(Timestep timestep, EditorCamera& camera);

		void OnPauseStart();
		void OnPauseUpdate(Timestep timestep, EditorCamera& camera);
		void OnStep(int steps);

		void OnEditorUpdate(Timestep timestep, EditorCamera& camera);

	};
}