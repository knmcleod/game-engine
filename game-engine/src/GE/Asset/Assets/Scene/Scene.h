#pragma once

#include "Entity.h"
#include "Components/Components.h"

#include "GE/Asset/Assets/Asset.h"

#include "GE/Core/Events/Event.h"

#include <glm/glm.hpp>

class b2World;

namespace GE
{
	// Forward declaration
	class Project;

	class Scene : public Asset
	{
		friend class Project;
	public:
		enum class State
		{
			Stop = 0,
			Run,
			Pause	// Set during Run
		};

		/*
		* Contains
		* - ViewportWidth : uint32_t
		* - ViewportHeight : uint32_t
		* - CurrentState : uint32_t
		* - StepFrames : uint64_t
		*/
		struct Config
		{
		public:
			Config() = default;
			Config(State state, const uint32_t& w, const uint32_t& h) : CurrentState(state)
			{
				SetViewport(w, h);
			}

			void SetViewport(const uint32_t& w, const uint32_t& h)
			{
				if ((w == 0 || h == 0) || (w == ViewportWidth && h == ViewportHeight)) return;
				ViewportWidth = w; ViewportHeight = h;
			}
		public:
			uint32_t ViewportWidth = 1280, ViewportHeight = 720;
			State CurrentState = State::Stop;
			uint64_t StepFrames = 0;
		};

		template<typename T>
		void OnEntityComponentAdded(Entity entity, T& component);

		/*
		* Returns all Entities in Scene with Component of type T
		* T : Component Type to get
		*/
		template<typename... T>
		std::vector<Entity> GetAllEntitiesWith()
		{
			std::vector<Entity> entities = std::vector<Entity>();
			auto& view = m_Registry.view<T...>();
			for (entt::entity internalEntity : view)
			{
				Entity entity = Entity((uint32_t)internalEntity);
				entities.push_back(entity);
			}
			return entities;
		}

		/*
		* Adds or replaces Component of type T using args
		*/
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Entity entity, Args&&... args)
		{
			T& component = m_Registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
			OnEntityComponentAdded<T>(entity, component);
			return component;
		}

		template<typename T>
		bool HasComponent(Entity entity) const
		{
			if (!entity)
				return false;
			return m_Registry.all_of<T>(entity);
		}

		template<typename T, typename... Args>
		T& AddComponent(Entity entity, Args&&... args)
		{
			if (HasComponent<T>(entity))
			{
				GE_CORE_WARN("Scene::AddComponent(Entity, Args&&...) - Failed to add Component to Entity.\n\tComponent already exists on Entity.");
				return GetComponent<T>(entity);
			}
			T& component = m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
			OnEntityComponentAdded<T>(entity, component);
			return component;
		}

		template<typename T>
		T& GetComponent(Entity entity)
		{
			GE_CORE_ASSERT(HasComponent<T>(entity), "Scene::GetComponent(Entity) - Component doesn't exist on Entity!");
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

		Scene() = default;
		Scene(UUID handle, const Config& config = Config());
		~Scene() override;

		/*
		*	Returns Copy of Scene Asset by CreateRef<Scene>()
		*/
		Ref<Asset> GetCopy() override;

		const entt::registry& GetRegistry() { return m_Registry; }

		const Config& GetConfig() const { return m_Config; }
		const State& GetState() const { return m_Config.CurrentState; }

		// Returns true if the scenes state is Run. Does not account for Simulation
		bool IsRunning() const { return m_Config.CurrentState == State::Run; }
		bool IsPaused() const { return m_Config.CurrentState == State::Pause; }
		bool IsStopped() const { return m_Config.CurrentState == State::Stop; }

		bool EntityExists(Entity entity);
		/*
		* Returns first primary Camera Entity that handles given LayerID
		*	otherwise, returns empty Entity
		*/
		Entity GetPrimaryCameraEntity(uint64_t layerID);
		/*
		* Returns first GUICanvas Entity that 
		* - handles given LayerID
		* - is active & not hidden
		*	otherwise, returns empty Entity
		*/
		Entity GetGUICanvasEntity(uint64_t layerID);
		Entity GetEntityByUUID(UUID uuid);
		Entity GetEntityByName(const std::string& name);

		Entity CreateEntity(const std::string& name, uint32_t tagID);
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name, uint32_t tagID);

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
		/*
		* Erases entity & its children from registry
		*
		* @param entity : entity to erase
		*/
		void DestroyEntity(Entity entity);

		/*
		* Adds translation & rotation offset recursively 
		* from self to root entity using RelationshipComponent.Parent until self is root
		* 
		* @param entity : entity to get offset from
		* @param outTranslationOffset : entity translation offset. Set inside method.
		* @param outRotationOffset : entity rotation offset. Set inside method.
		*/
		void GetTotalOffset(Entity entity, glm::vec3& outTranslationOffset, glm::vec3& outRotationOffset);

		/*
		* Returns all Entities that are valid for rendering using RenderComponent::LayerIDs
		*
		* @param layerID : ID of layer that plans to render entities
		*/
		std::vector<Entity> GetAllRenderEntities(const uint64_t& layerID);
		/*
		* If Entitys parent isn't self, removes self from parent & sets new parent
		* 
		* @param child : entity to handle
		* @param parentID : IDComponent::ID of new parent
		*/
		void SetEntityParent(Entity child, const UUID& parentID);

		/*
		* Sets Entity Camera position & rotation
		*/
		void SyncCamera(Entity entity, const glm::vec3& position, const glm::vec3& rotation);
	
		/*
		* Resizes Scene viewport & all Entity Camera viewports
		*/
		void OnResizeViewport(uint32_t width, uint32_t height);

private:
		/*
		* Gets all Entities with RenderComponent & sets Rendered to false
		* Meant to be called prior to Layer::OnUpdate()
		*/
		void ResetEntityRenderComponents();

		void OnStart(State state, uint32_t viewportWidth = 0, uint32_t viewportHeight = 0);
		void OnUpdate(Timestep ts);
		void OnStop();
		void OnStep(int steps);

		/*
		* Calls Scripting Event for Entity if Scene is Running || Paused.
		* Returns if Event is handled by GE-ScriptCore::Entity::OnEvent(Event&)
		* @param e : event to handle
		* @param entity : entity to call scripting events for
		*/
		bool OnEvent(Event& e, Entity entity);

		void OnRuntimeStart();
		void OnRuntimeUpdate(Timestep ts);

		void OnPauseStart();
		void OnPauseUpdate(Timestep ts);

		void InitializePhysics2D();
		void UpdatePhysics2D(Timestep ts);
		void DestroyPhysics2D();

		void InitializeScripting();
		void UpdateScripting(Timestep ts);
		void DestroyScripting();

		void InitializeAudio();
		void UpdateAudio(Timestep ts);
		void DestroyAudio();
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
			case Scene::State::Stop:
				return "Stop";
			case Scene::State::Run:
				return "Run";
			case Scene::State::Pause:
				return "Pause";
			}

			GE_CORE_ERROR("Invalid Scene Type.");
			return "<Not Found>";
		}
	}
}