#pragma once

#include "GE/Asset/Asset.h"
#include "GE/Audio/AudioManager.h"

#include "GE/Core/Time/Time.h"
#include "GE/Core/UUID/UUID.h"

#include "GE/Rendering/Camera/Editor/EditorCamera.h"

#include <entt/entt.hpp>

class b2World;

namespace GE
{
	enum class SceneState
	{
		Run = 0,
		Simulate = 1,
		Stop = 2,
		Pause = 3 // Called during Run or Simulate
	};

	namespace SceneUtils
	{
		static const std::string& SceneStateToString(SceneState state)
		{
			switch (state)
			{
			case SceneState::Run:
				return "Run";
			case SceneState::Simulate:
				return "Simulate";
			case SceneState::Pause:
				return "Pause";
			case SceneState::Stop:
				return "Stop";
			}

			GE_CORE_ERROR("Invalid Scene Type.");
			return "<Not Found>";
		}
	}

	class Scene : public Asset
	{
		friend class AssetSerializer;
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class EditorLayer;
	private:
		template<typename T>
		void OnComponentAdded(Entity entity);

		std::string m_Name = "Scene";
		SceneState m_SceneState = SceneState::Stop;
		int m_StepFrames = 0;
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;
		Ref<AudioManager> m_AudioManager = nullptr;

		void InitializeAudio();
		void UpdateAudio(Timestep timestep);
		void DestroyAudio();

		void InitializePhysics2D();
		void UpdatePhysics2D(Timestep timestep);
		void DestroyPhysics2D();

		void InitializeScripting();
		void UpdateScripting(Timestep timestep);
		void DestroyScripting();
	public:
		static Ref<Scene> Copy(const Ref<Scene> scene);
		static AssetType GetAssetType() { return AssetType::Scene; }

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		Scene();
		Scene(const std::string& name);
		~Scene();
		
		std::string& GetName() { return m_Name; }
		void SetName(std::string name) { m_Name = name; }

		// Returns true if the scenes state is Run. Does not account for Simulation
		bool IsRunning() const { return m_SceneState == SceneState::Run; }
		bool IsPaused() const { return m_SceneState == SceneState::Pause; }

		void Render();
		void Render(const EditorCamera& camera);

		void OnEntitySelected(Entity entity);

		Entity GetPrimaryCameraEntity();
		Entity GetEntityByUUID(UUID uuid);
		Entity GetEntityByTag(const std::string& tag);

		Entity CreateEntity(const std::string& name);
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name);
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

		// Asset override
		virtual AssetType GetType() override { return GetAssetType(); }
	};
}