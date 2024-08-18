#pragma once

#include "GE/Asset/Assets/Asset.h"

#include "GE/Rendering/Camera/Camera.h"

#include <entt/entt.hpp>

class b2World;

namespace GE
{
	class Scene;

	class Entity
	{
		friend class Scene;
	public:
		operator entt::entity() const { return m_EntityID; }
		operator bool() const { return m_EntityID != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityID; }

		bool operator ==(const Entity& other) const { return m_EntityID == other.m_EntityID; }
		bool operator !=(const Entity& other) const { return !operator==(other); }

		template<typename T>
		void OnComponentAdded();

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			GE_CORE_ASSERT(m_Scene, "Entity doesn't have Scene.");
			return m_Scene->AddOrReplaceComponent<T, Args...>(*this, args...);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			GE_CORE_ASSERT(m_Scene, "Entity doesn't have Scene.");
			return m_Scene->AddComponent<T, Args...>(*this, args...);
		}

		template<typename T>
		T& GetComponent() const
		{
			GE_CORE_ASSERT(m_Scene, "Entity doesn't have Scene.");
			return m_Scene->GetComponent<T>(*this);
		}

		template<typename T, typename... Args>
		T& GetOrAddComponent(Args&&... args)
		{
			if (!HasComponent<T>())
				return AddComponent<T>();
			return GetComponent<T>();
		}

		template<typename T>
		void RemoveComponent()
		{
			GE_CORE_ASSERT(m_Scene, "Entity doesn't have Scene.");
			m_Scene->RemoveComponent<T>(*this);
		}

		template<typename T>
		bool HasComponent() const
		{
			GE_CORE_ASSERT(m_Scene, "Entity doesn't have Scene.");
			return m_Scene->HasComponent<T>(*this);
		}

		Entity() = default;
		Entity(entt::entity entityID, Scene* scene = nullptr);
		virtual ~Entity();

		const uint32_t GetEntityID() const { return (uint32_t)m_EntityID; }
		const Scene* GetScene() { return m_Scene; }
	protected:
		void ClearEntityID() { m_EntityID = entt::null; }
	protected:
		entt::entity m_EntityID{ entt::null };
		Scene* m_Scene = nullptr;
	};

	class ScriptableEntity : public Entity
	{
		friend class Scene;
	public:
		operator entt::entity() const { return m_EntityID; }
		operator bool() const { return m_EntityID != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityID; }

		bool operator ==(const Entity& other) const { return this->operator==(other); }
		bool operator !=(const Entity& other) const { return !operator==(other); }

		ScriptableEntity(entt::entity id, Scene* scene) : Entity(id, scene)
		{

		}

		virtual ~ScriptableEntity() override = 0;

		void SetID(uint32_t id, Ref<Scene> scene = nullptr)
		{
			m_EntityID = (entt::entity)id;
			if (scene != nullptr && m_Scene != scene.get())
				m_Scene = scene.get();
		}

	protected:
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;
		virtual void OnUpdate(Timestep timestep) = 0;
	};

	class Scene : public Asset
	{
		friend class AssetSerializer;
	public:
		enum class State
		{
			Run = 0,
			Simulate = 1,
			Stop = 2,
			Pause = 3 // Set during Run or Simulate
		};

		struct Config
		{
			// Remove. Use AssetMetadata.FilePath.Name instead
			std::string Name = "NewScene";
			uint32_t ViewportWidth = 1280, ViewportHeight = 720;
			State State = State::Stop;
			int StepFrames = 0;
		};

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Entity entity, Args&&... args)
		{
			T& component = m_Registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
			entity.OnComponentAdded<T>();
			return component;
		}

		template<typename T>
		bool HasComponent(Entity entity) const
		{
			return m_Registry.all_of<T>(entity);
		}

		template<typename T, typename... Args>
		T& AddComponent(Entity entity, Args&&... args)
		{
			GE_CORE_ASSERT(!HasComponent<T>(entity), "Component already exists on Entity!");
			T& component = m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
			entity.OnComponentAdded<T>();
			return component;
		}

		template<typename T>
		T& GetComponent(Entity entity)
		{
			GE_CORE_ASSERT(HasComponent<T>(entity), "Component doesn't exist on Entity!");
			return m_Registry.get<T>(entity);
		}

		template<typename T, typename... Args>
		T& GetOrAddComponent(Entity entity, Args&&... args)
		{
			if (!HasComponent<T>(entity))
				return AddComponent<T, Args&&...>(entity, args...);
			return GetComponent<T>(entity);
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			GE_CORE_ASSERT(HasComponent<T>(entity), "Component doesn't exist on Entity!");
			m_Registry.remove<T>(entity);
		}

		Scene();
		Scene(UUID handle, const std::string& name = std::string());
		~Scene() override;

		/*
		*	Returns Copy of Asset by CreateRef<this>()
		*	By default, returns nullptr.
		*	Implement per inherited Asset
		*/
		Ref<Asset> GetCopy() override;
		
		// Returns true if the scenes state is Run. Does not account for Simulation
		bool IsRunning() const { return m_Config.State == State::Run; }
		bool IsPaused() const { return m_Config.State == State::Pause; }
		bool IsStopped() const { return m_Config.State == State::Stop; }

		const Config& GetConfig() const { return m_Config; }
		const std::string& GetName() const { return m_Config.Name; }
		const State& GetState() const { return m_Config.State; }

		const entt::registry& GetRegistry() { return m_Registry; }

		Entity GetPrimaryCameraEntity();
		Entity GetEntityByUUID(UUID uuid);
		Entity GetEntityByTag(const std::string& tag);

		Entity CreateEntity(const std::string& name);
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name);
		/*
		* returns duplicated Entity.
		* Will not share UUID
		*/
		Entity DuplicateEntity(Entity entity);
		/*
		* returns copied Entity.
		* Will share UUID
		*/
		Entity CopyEntity(Entity entity);
		void DestroyEntity(Entity entity);

		void OnStart(State state, uint32_t viewportWidth = 0, uint32_t viewportHeight = 0);
		void OnUpdate(Timestep ts, Camera* camera = nullptr);
		void OnStop();
		void OnStep(int steps);

	private:
		void OnResizeViewport(uint32_t width, uint32_t height);

		void OnRuntimeStart();
		void OnRuntimeUpdate(Timestep timestep);

		void OnSimulationStart();
		void OnSimulationUpdate(Timestep timestep, Camera* camera = nullptr);

		void OnPauseStart();
		void OnPauseUpdate(Timestep timestep, Camera* camera = nullptr);

		/*
		* Updates camera and calls Render
		*/
		void OnEditorUpdate(Timestep timestep, Camera* camera = nullptr);

		/*
		* Renders using first found Primary Camera via Component
		*/
		void Render();
		/*
		* Updates scene viewport using camera and Renders
		*/
		void Render(Camera* camera);

		void InitializePhysics2D();
		void UpdatePhysics2D(Timestep timestep);
		void DestroyPhysics2D();

		void InitializeScripting();
		void UpdateScripting(Timestep timestep);
		void DestroyScripting();
	private:
		Config m_Config;
		entt::registry m_Registry;

		// TODO: Physics wrapper
		b2World* m_PhysicsWorld = nullptr;
	};

	namespace SceneUtils
	{
		static const std::string SceneStateToString(const Scene::State& state)
		{
			switch (state)
			{
			case Scene::State::Run:
				return "Run";
			case Scene::State::Simulate:
				return "Simulate";
			case Scene::State::Pause:
				return "Pause";
			case Scene::State::Stop:
				return "Stop";
			}

			GE_CORE_ERROR("Invalid Scene Type.");
			return "<Not Found>";
		}
	}
}