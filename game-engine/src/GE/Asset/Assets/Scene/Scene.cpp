#include "GE/GEpch.h"

#include "Scene.h"
#include "Components/Components.h"

#include "GE/Audio/AudioManager.h"
#include "GE/Core/Application/Application.h"
#include "GE/Physics/Physics.h"
#include "GE/Project/Project.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "GE/Scripting/Scripting.h"

namespace GE
{
	template<typename T>
	static void CopyComponent(entt::registry& to, entt::registry& from, const std::unordered_map<UUID, entt::entity>& entityMap)
	{
		auto view = from.view<T>();
		for (auto e : view)
		{
			UUID uuid = from.get<IDComponent>(e).ID;
			GE_CORE_ASSERT(entityMap.find(uuid) != entityMap.end(), "Could not find Entity in map.");
			entt::entity enttID = entityMap.at(uuid);
			auto& component = from.get<T>(e);
			to.emplace_or_replace<T>(enttID, component);
		}
	}

	template<typename T>
	static void CopyComponentIfExists(Entity to, Entity from)
	{
		if (from.HasComponent<T>())
		{
			T component = from.GetComponent<T>();
			to.AddOrReplaceComponent<T>(component);
		}
	}

#pragma region Scene

	Scene::Scene(UUID handle, const Config& config /*= Config()*/) : Asset(handle, Scene::Type::Scene)
	{
		m_Config = Config(config);
	}

	Scene::~Scene()
	{
		m_Registry.clear();
	}

	Ref<Asset> Scene::GetCopy()
	{
		Ref<Scene> newScene = CreateRef<Scene>(this->p_Handle, this->m_Config);

		auto& sceneRegistry = this->m_Registry;
		auto& newSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> entityMap;

		// Copy Components
		{
			GE_PROFILE_SCOPE("Scene - Copy : Components");

			// Special Case - Find/Set UUID, Name & TagID
			auto idView = sceneRegistry.view<IDComponent>();
			for (auto e : idView)
			{
				UUID uuid = sceneRegistry.get<IDComponent>(e).ID;
				const auto& name = sceneRegistry.get<NameComponent>(e).Name;
				uint32_t tagID = sceneRegistry.get<TagComponent>(e).ID;
				Entity entity = newScene->CreateEntityWithUUID(uuid, name, tagID);
				entityMap.emplace(uuid, entity);
			}

			CopyComponent<TransformComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<AudioSourceComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<AudioListenerComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<RenderComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CameraComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<SpriteRendererComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CircleRendererComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<TextRendererComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<NativeScriptComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<ScriptComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<Rigidbody2DComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<BoxCollider2DComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CircleCollider2DComponent>(newSceneRegistry, sceneRegistry, entityMap);
		}

		return newScene;
	}

	void Scene::OnResizeViewport(uint32_t width, uint32_t height)
	{
		if ((width == 0 || height == 0))
		{
			GE_CORE_WARN("Scene failed to resize viewport\n\tWidth: {0}\n\tHeight: {1}", width, height);
			return;
		}
		m_Config.SetViewport(width, height);

		// Resize non-fixed aspect ratio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			if (!camera.FixedAspectRatio)
			{
				auto& c = camera.ActiveCamera;
				SyncEntityCamera(&c);
			}
		}
	}

	void Scene::OnStart(State state, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		OnResizeViewport(viewportWidth, viewportHeight);

		switch (state)
		{
		case State::Stop:
			OnStop();
			break;
		case State::Run:
			OnRuntimeStart();
			break;
		case State::Pause:
			OnPauseStart();
			break;
		default:
			GE_CORE_INFO("Cannot Start Scene.\n\tState: {0}", SceneUtils::SceneStateToString(state));
			break;
		}
	}

	void Scene::OnUpdate(Timestep ts)
	{
		ResetEntityRenderComponents();

		switch (m_Config.CurrentState)
		{
		case State::Run:
			OnRuntimeUpdate(ts);
			break;
		case State::Pause:
			OnPauseUpdate(ts);
			break;
		default:
			break;
		}
	}

	void Scene::OnStop()
	{
		m_Config.CurrentState = State::Stop;
		
		DestroyScripting();
		DestroyAudio();
		DestroyPhysics2D();
	}

	void Scene::OnStep(int steps)
	{
		m_Config.StepFrames += steps;
	}

	bool Scene::OnEvent(Event& e, Entity entity) const
	{
		if (!IsStopped())
		{
			return Scripting::OnEvent(e, entity);
		}
		return false;
	}

	void Scene::ResetEntityRenderComponents()
	{
		std::vector<Entity> entities = GetAllEntitiesWith<RenderComponent>();
		for (Entity entity : entities)
		{
			entity.GetComponent<RenderComponent>().Rendered = false;
		}
		entities.clear();
		entities = std::vector<Entity>();
	}

	void Scene::OnRuntimeStart()
	{
		m_Config.CurrentState = State::Run;

		InitializePhysics2D();
		InitializeAudio();
		InitializeScripting();
	}
	
	void Scene::OnRuntimeUpdate(Timestep ts)
	{
		UpdatePhysics2D(ts);
		UpdateScripting(ts);
		UpdateAudio(ts);
	}

	void Scene::OnPauseStart()
	{
		m_Config.CurrentState = State::Pause;

	}

	void Scene::OnPauseUpdate(Timestep ts)
	{	
		if (m_Config.StepFrames > 0)
		{
			UpdatePhysics2D(ts);
			UpdateScripting(ts);
			UpdateAudio(ts);

			m_Config.StepFrames--;
		}
	}

#pragma region Physics

	void Scene::InitializePhysics2D()
	{
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
		std::vector<Entity> entities = GetAllEntitiesWith<Rigidbody2DComponent>();
		for (Entity entity : entities)
		{
			auto& tc = GetComponent<TransformComponent>(entity);
			auto& rb2D = GetComponent<Rigidbody2DComponent>(entity);

			b2BodyDef bodyDef;
			bodyDef.type = Physics::Rigidbody2DTypeToBox2DBody(rb2D.Type);
			bodyDef.position.Set(tc.Translation.x, tc.Translation.y);
			bodyDef.angle = tc.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2D.FixedRotation);

			rb2D.RuntimeBody = body;

			//	Box Collider
			{
				GE_PROFILE_SCOPE("Scene - InitializePhysics2D : Box Collider");
				if (HasComponent<BoxCollider2DComponent>(entity))
				{
					auto& bc2D = GetComponent<BoxCollider2DComponent>(entity);

					b2PolygonShape polygonShape;
					polygonShape.SetAsBox(bc2D.Size.x * tc.Scale.x,
						bc2D.Size.y * tc.Scale.y);

					b2FixtureDef fixtureDef;
					fixtureDef.shape = &polygonShape;
					fixtureDef.density = bc2D.Density;
					fixtureDef.friction = bc2D.Friction;
					fixtureDef.restitution = bc2D.Restitution;
					fixtureDef.restitutionThreshold = bc2D.RestitutionThreshold;

					body->CreateFixture(&fixtureDef);
				}
			}

			//	Circle Collider
			{
				GE_PROFILE_SCOPE("Scene - InitializePhysics2D : Circle Collider");
				if (HasComponent<CircleCollider2DComponent>(entity))
				{
					auto& cc2D = GetComponent<CircleCollider2DComponent>(entity);

					b2CircleShape circleShape;
					circleShape.m_p.Set(cc2D.Offset.x, cc2D.Offset.y);
					circleShape.m_radius = tc.Scale.x * cc2D.Radius;

					b2FixtureDef fixtureDef;
					fixtureDef.shape = &circleShape;
					fixtureDef.density = cc2D.Density;
					fixtureDef.friction = cc2D.Friction;
					fixtureDef.restitution = cc2D.Restitution;
					fixtureDef.restitutionThreshold = cc2D.RestitutionThreshold;

					body->CreateFixture(&fixtureDef);
				}
			}
		}
		entities.clear();
		entities = std::vector<Entity>();
	}
	
	void Scene::UpdatePhysics2D(Timestep ts)
	{
		GE_PROFILE_SCOPE("Scene - UpdatePhysics2D");
		if (m_PhysicsWorld)
		{
			const int32_t velocityInteration = 5;
			const int32_t positionInteration = 5;
			m_PhysicsWorld->Step(ts, velocityInteration, positionInteration);

			std::vector<Entity> entities = GetAllEntitiesWith<Rigidbody2DComponent>();
			for (Entity entity : entities)
			{
				auto& transformComponent = GetComponent<TransformComponent>(entity);
				auto& rb2D = GetComponent<Rigidbody2DComponent>(entity);

				GE_CORE_ASSERT(rb2D.RuntimeBody != nullptr, "Rigidbody2DComponent has no Runtime Body.");
				b2Body* body = (b2Body*)rb2D.RuntimeBody;
				const auto& position = body->GetPosition();
				transformComponent.Translation.x = position.x;
				transformComponent.Translation.y = position.y;

				transformComponent.Rotation.z = body->GetAngle();
			}
			entities.clear();
			entities = std::vector<Entity>();
		}
	}

	void Scene::DestroyPhysics2D()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}
#pragma endregion

#pragma region Scripting
	void Scene::InitializeScripting()
	{
		GE_PROFILE_SCOPE("InitializeScripting");

		std::vector<Entity> nscEntities = GetAllEntitiesWith<NativeScriptComponent>();
		for (Entity entity : nscEntities)
		{
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			if (!nsc.Instance)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->SetID(entity);
				nsc.Instance->OnCreate();
			}
		}
		nscEntities.clear();
		nscEntities = std::vector<Entity>();

		std::vector<Entity> scEntities = GetAllEntitiesWith<ScriptComponent>();
		for (Entity entity : scEntities)
		{
			Scripting::OnCreateScript(entity);
		}
		scEntities.clear();
		scEntities = std::vector<Entity>();
	}

	void Scene::UpdateScripting(Timestep ts)
	{
		GE_PROFILE_SCOPE("UpdateScripting");

		std::vector<Entity> nscEntities = GetAllEntitiesWith<NativeScriptComponent>();
		for (Entity entity : nscEntities)
		{
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.Instance->OnUpdate(ts);
		}
		nscEntities.clear();
		nscEntities = std::vector<Entity>();

		std::vector<Entity> scEntities = GetAllEntitiesWith<ScriptComponent>();
		for (Entity entity : scEntities)
		{
			Scripting::OnUpdateScript(entity, ts);
		}
		scEntities.clear();
		scEntities = std::vector<Entity>();

	}

	void Scene::DestroyScripting()
	{
		GE_PROFILE_SCOPE("DestroyScripting");
		Scripting::OnStop();
	}
#pragma endregion

#pragma region Audio
	void Scene::InitializeAudio() 
	{
		std::vector<Entity> entities = GetAllEntitiesWith<AudioSourceComponent>();
		for (Entity entity : entities)
		{
			auto& asc = GetComponent<AudioSourceComponent>(entity);
			auto& trsc = GetComponent<TransformComponent>(entity);
			
			glm::vec3 velocity = glm::vec3(0.0);
			// TODO : check for physics RB and assign velocity if possible
			AudioManager::GenerateSource(asc, trsc.Translation, velocity);
		}
		entities.clear();
		entities = std::vector<Entity>();
	}

	void Scene::UpdateAudio(Timestep ts)
	{
		std::vector<Entity> listenerEntities = GetAllEntitiesWith<AudioListenerComponent>();
		for (Entity entity : listenerEntities)
		{
			auto& alc = GetComponent<AudioListenerComponent>(entity);
			auto& trsc = GetComponent<TransformComponent>(entity);

			glm::vec3 velocity = glm::vec3(0.0);
			if (HasComponent<Rigidbody2DComponent>(entity))
			{
				auto& rb2D = GetComponent<Rigidbody2DComponent>(entity);
				if (rb2D.RuntimeBody)
				{
					b2Body* body = (b2Body*)rb2D.RuntimeBody;
					const auto& v = body->GetLinearVelocity();
					velocity = glm::vec3(v.x, v.y, 0.0);
				}
			}

			AudioManager::UpdateListener(alc, trsc.Translation, velocity);
		}
		listenerEntities.clear();
		listenerEntities = std::vector<Entity>();

		std::vector<Entity> sourceEntities = GetAllEntitiesWith<AudioSourceComponent>();
		for (Entity entity : sourceEntities)
		{
			auto& asc = GetComponent<AudioSourceComponent>(entity);
			auto& trsc = GetComponent<TransformComponent>(entity);

			glm::vec3 velocity = glm::vec3(0.0);
			if (HasComponent<Rigidbody2DComponent>(entity))
			{
				auto& rb2D = GetComponent<Rigidbody2DComponent>(entity);
				if (rb2D.RuntimeBody)
				{
					b2Body* body = (b2Body*)rb2D.RuntimeBody;
					const auto& v = body->GetLinearVelocity();
					velocity = glm::vec3(v.x, v.y, 0.0);
				}
			}

			AudioManager::UpdateSource(asc, trsc.Translation, velocity);
		}
		sourceEntities.clear();
		sourceEntities = std::vector<Entity>();
	}

	void Scene::DestroyAudio()
	{
		std::vector<Entity> entities = GetAllEntitiesWith<AudioSourceComponent>();
		for (Entity entity : entities)
		{
			auto& asc = GetComponent<AudioSourceComponent>(entity);
			
			AudioManager::RemoveSource(asc.ID);
		}
		entities.clear();
		entities = std::vector<Entity>();
	}
#pragma endregion

#pragma region Entity Control

	template<typename T>
	void Scene::OnEntityComponentAdded(Entity e)
	{
		e.OnComponentAdded<T>();
	}

	template<>
	void Scene::OnEntityComponentAdded<CameraComponent>(Entity e)
	{
		SyncEntityCamera(&GetComponent<CameraComponent>(e).ActiveCamera);
		e.OnComponentAdded<CameraComponent>();
	}

	bool Scene::EntityExists(Entity e)
	{
		return m_Registry.any_of(e);
	}

	Entity Scene::GetPrimaryCameraEntity(uint64_t layerID)
	{
		Entity retEntity = {};
		std::vector<Entity> entities = GetAllEntitiesWith<CameraComponent, RenderComponent>();
		for (Entity entity : entities)
		{
			const auto& cc = entity.GetComponent<CameraComponent>();
			const auto& rc = entity.GetComponent<RenderComponent>();
			if (cc.Primary && rc.IDHandled(layerID))
				retEntity = entity;
		}
		entities.clear();
		entities = std::vector<Entity>();
		return retEntity;
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		Entity retEntity = {};
		if (uuid != 0)
		{
			std::vector<Entity> entities = GetAllEntitiesWith<IDComponent>();
			for (Entity entity : entities)
			{
				const auto& idc = entity.GetComponent<IDComponent>();
				if (uuid == idc.ID)
					retEntity = entity;
			}
			entities.clear();
			entities = std::vector<Entity>();
		}
		return retEntity;
	}

	Entity Scene::GetEntityByName(const std::string& name)
	{
		std::vector<Entity> entities = GetAllEntitiesWith<NameComponent>();
		for (Entity entity : entities)
		{
			const auto& nc = entity.GetComponent<NameComponent>();
			if (name == nc.Name)
				return entity;
		}
		return { };
	}

	void Scene::SyncEntityCamera(Camera* camera) const
	{
		const glm::vec2 vp = camera->GetViewport();
		if (vp.x != m_Config.ViewportWidth && vp.y != m_Config.ViewportHeight)
			camera->SetViewport(m_Config.ViewportWidth, m_Config.ViewportWidth);
	}

	Entity Scene::CreateEntity(const std::string& name, uint32_t tagID)
	{
		return CreateEntityWithUUID(UUID(), name, tagID);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, uint32_t tagID)
	{
		entt::entity internalEntity = m_Registry.create();
		Entity entity = Entity((uint32_t)internalEntity, this);

		AddComponent<IDComponent>(entity, uuid);
		AddComponent<TagComponent>(entity, tagID);
		AddComponent<NameComponent>(entity, name);
		AddComponent<TransformComponent>(entity);
		AddComponent<RenderComponent>(entity, false);
		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetComponent<NameComponent>().Name;
		uint32_t tagID = entity.GetComponent<TagComponent>().ID;
		Entity newEntity = CreateEntity(name, tagID);

		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<RenderComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
		CopyComponentIfExists<TextRendererComponent>(newEntity, entity);
		CopyComponentIfExists<AudioSourceComponent>(newEntity, entity);
		CopyComponentIfExists<AudioListenerComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
		CopyComponentIfExists<ScriptComponent>(newEntity, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(newEntity, entity);

		return newEntity;
	}

	Entity Scene::CopyEntity(Entity entity)
	{
		Entity newEntity = CreateEntityWithUUID(entity.GetComponent<IDComponent>().ID, 
			entity.GetComponent<NameComponent>().Name, entity.GetComponent<TagComponent>().ID);

		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<RenderComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
		CopyComponentIfExists<TextRendererComponent>(newEntity, entity);
		CopyComponentIfExists<AudioSourceComponent>(newEntity, entity);
		CopyComponentIfExists<AudioListenerComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
		CopyComponentIfExists<ScriptComponent>(newEntity, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(newEntity, entity);

		return newEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
		entity.ClearEntityID();
	}

#pragma endregion

#pragma endregion
}