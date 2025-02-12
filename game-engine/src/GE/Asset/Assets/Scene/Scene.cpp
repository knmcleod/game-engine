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
	static void CopyComponentIfExists(Scene* scene, Entity to, Entity from)
	{
		if (scene && scene->HasComponent<T>(from))
		{
			T& component = scene->GetComponent<T>(from);
			scene->AddOrReplaceComponent<T>(to, component);
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
				uint32_t tagID = sceneRegistry.get<TagComponent>(e).TagID;
				Entity entity = newScene->CreateEntityWithUUID(uuid, name, tagID);
				entityMap.emplace(uuid, entity);
			}

			CopyComponent<ActiveComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<RelationshipComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<TransformComponent>(newSceneRegistry, sceneRegistry, entityMap);
			
			CopyComponent<AudioSourceComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<AudioListenerComponent>(newSceneRegistry, sceneRegistry, entityMap);
			
			CopyComponent<RenderComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CameraComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<SpriteRendererComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CircleRendererComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<TextRendererComponent>(newSceneRegistry, sceneRegistry, entityMap);

			CopyComponent<GUIComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<GUICanvasComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<GUILayoutComponent>(newSceneRegistry, sceneRegistry, entityMap);
			// TODO : GUIMaskComponent
			CopyComponent<GUIImageComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<GUIButtonComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<GUIInputFieldComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<GUISliderComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<GUICheckboxComponent>(newSceneRegistry, sceneRegistry, entityMap);
			// TODO : GUIScrollRectComponent & GUIScrollbarComponent
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
		if (width == 0 || height == 0)
		{
			GE_CORE_WARN("Scene failed to resize viewport\n\tWidth: {0}\n\tHeight: {1}", width, height);
			return;
		}
		m_Config.SetViewport(width, height);

		// Resize non-fixed aspect ratio cameras
		std::vector<Entity> entities = GetAllEntitiesWith<CameraComponent>();
		for (Entity entity : entities)
		{
			UUID uuid = GetComponent<IDComponent>(entity).ID;
			auto& cc = GetComponent<CameraComponent>(entity);
			if (!cc.FixedAspectRatio)
			{
				cc.ActiveCamera.SetViewport(m_Config.ViewportWidth, m_Config.ViewportHeight);
			}
		}

		entities.clear();
		entities = std::vector<Entity>();
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
			GE_CORE_ERROR("Cannot Start Scene with State::{0}.", SceneUtils::SceneStateToString(state));
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

	bool Scene::OnEvent(Event& e, Entity entity)
	{
		if (!IsStopped())
		{
			return Scripting::OnEvent(e, this, entity);
		}
		return false;
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
		GE_PROFILE_FUNCTION();

		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
		std::vector<Entity> entities = GetAllEntitiesWith<Rigidbody2DComponent>();
		for (Entity entity : entities)
		{
			auto& trsc = GetComponent<TransformComponent>(entity);
			auto& rb2D = GetComponent<Rigidbody2DComponent>(entity);
			const glm::vec3& translation = trsc.GetOffsetTranslation();

			//	Box Collider
			{
				GE_PROFILE_SCOPE("Scene - InitializePhysics2D : Box Collider");
				if (HasComponent<BoxCollider2DComponent>(entity))
				{
					auto& bc2D = GetComponent<BoxCollider2DComponent>(entity);

					b2BodyDef bodyDef;
					bodyDef.type = Physics::Rigidbody2DTypeToBox2DBody(rb2D.Type);
					// TODO : Offset for Middle Pivots(?)
					bodyDef.position.Set(translation.x, translation.y);
					bodyDef.angle = trsc.Rotation.z;

					b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
					body->SetFixedRotation(rb2D.FixedRotation);
					rb2D.RuntimeBody = body;

					float halfW = (bc2D.Size.x * trsc.Scale.x) * 0.5f;
					float halfH = (bc2D.Size.y * trsc.Scale.y) * 0.5f;

					b2PolygonShape polygonShape;
					polygonShape.SetAsBox(halfW, halfH);

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

					b2BodyDef bodyDef;
					bodyDef.type = Physics::Rigidbody2DTypeToBox2DBody(rb2D.Type);
					bodyDef.position.Set(translation.x, translation.y);
					bodyDef.angle = trsc.Rotation.z;

					b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
					body->SetFixedRotation(rb2D.FixedRotation);

					rb2D.RuntimeBody = body;

					b2CircleShape circleShape;
					
					circleShape.m_p.Set(cc2D.Offset.x, cc2D.Offset.y);
					circleShape.m_radius = cc2D.Radius * trsc.Scale.x;
		
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
		GE_PROFILE_FUNCTION();
		if (m_PhysicsWorld)
		{
			const int32_t velocityInteration = 5;
			const int32_t positionInteration = 5;
			m_PhysicsWorld->Step(ts, velocityInteration, positionInteration);

			std::vector<Entity> entities = GetAllEntitiesWith<Rigidbody2DComponent>();
			for (Entity entity : entities)
			{
				auto& trsc = GetComponent<TransformComponent>(entity);
				auto& rb2D = GetComponent<Rigidbody2DComponent>(entity);

				GE_CORE_ASSERT(rb2D.RuntimeBody != nullptr, "Rigidbody2DComponent has no Runtime Body.");
				b2Body* body = (b2Body*)rb2D.RuntimeBody;
				const auto& position = body->GetPosition();
				trsc.Translation.x = position.x;
				trsc.Translation.y = position.y;
				trsc.Rotation.z = glm::degrees(body->GetAngle());
			}
			entities.clear();
			entities = std::vector<Entity>();
		}
	}

	void Scene::DestroyPhysics2D()
	{
		GE_PROFILE_FUNCTION();
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}
#pragma endregion

#pragma region Scripting
	void Scene::InitializeScripting()
	{
		GE_PROFILE_FUNCTION();

		std::vector<Entity> nscEntities = GetAllEntitiesWith<NativeScriptComponent>();
		for (Entity entity : nscEntities)
		{
			auto& nsc = GetComponent<NativeScriptComponent>(entity);
			if (!nsc.Instance)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->p_EntityID = entity;
				nsc.Instance->OnCreate();
			}
		}
		nscEntities.clear();
		nscEntities = std::vector<Entity>();

		std::vector<Entity> scEntities = GetAllEntitiesWith<ScriptComponent>();
		for (Entity entity : scEntities)
		{
			Scripting::OnCreateScript(this, entity);
		}
		scEntities.clear();
		scEntities = std::vector<Entity>();
	
	}

	void Scene::UpdateScripting(Timestep ts)
	{
		GE_PROFILE_FUNCTION();

		std::vector<Entity> nscEntities = GetAllEntitiesWith<NativeScriptComponent>();
		for (Entity entity : nscEntities)
		{
			auto& nsc = GetComponent<NativeScriptComponent>(entity);
			nsc.Instance->OnUpdate(ts);
		}
		nscEntities.clear();
		nscEntities = std::vector<Entity>();

		std::vector<Entity> scEntities = GetAllEntitiesWith<ScriptComponent>();
		for (Entity entity : scEntities)
		{
			Scripting::OnUpdateScript(this, entity, ts);
		}
		scEntities.clear();
		scEntities = std::vector<Entity>();

	}

	void Scene::DestroyScripting()
	{
		GE_PROFILE_FUNCTION();
		Scripting::OnStop();
	}
#pragma endregion

#pragma region Audio
	void Scene::InitializeAudio() 
	{
		GE_PROFILE_FUNCTION();
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
		GE_PROFILE_FUNCTION();
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
		GE_PROFILE_FUNCTION();
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

#pragma region OnEntityComponentAdded
	template<typename T>
	void Scene::OnEntityComponentAdded(Entity entity, T& component)
	{
		entity.OnComponentAdded<T>();
	}

	template<>
	void Scene::OnEntityComponentAdded<CameraComponent>(Entity entity, CameraComponent& cc)
	{
		cc.ActiveCamera.SetViewport(m_Config.ViewportWidth, m_Config.ViewportHeight);
		entity.OnComponentAdded<CameraComponent>();
	}

	template<>
	void Scene::OnEntityComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent& rsc)
	{
		if(HasComponent<IDComponent>(entity))
			rsc.Parent = GetComponent<IDComponent>(entity).ID;
		entity.OnComponentAdded<RelationshipComponent>();
	}

#pragma region GUI
	template<>
	void Scene::OnEntityComponentAdded<GUICanvasComponent>(Entity entity, GUICanvasComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}
	template<>
	void Scene::OnEntityComponentAdded<GUIMaskComponent>(Entity entity, GUIMaskComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}
	template<>
	void Scene::OnEntityComponentAdded<GUIImageComponent>(Entity entity, GUIImageComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}
	template<>
	void Scene::OnEntityComponentAdded<GUIButtonComponent>(Entity entity, GUIButtonComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}
	template<>
	void Scene::OnEntityComponentAdded<GUIInputFieldComponent>(Entity entity, GUIInputFieldComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}
	template<>
	void Scene::OnEntityComponentAdded<GUISliderComponent>(Entity entity, GUISliderComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}
	template<>
	void Scene::OnEntityComponentAdded<GUICheckboxComponent>(Entity entity, GUICheckboxComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}

	template<>
	void Scene::OnEntityComponentAdded<GUIScrollRectComponent>(Entity entity, GUIScrollRectComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}
	template<>
	void Scene::OnEntityComponentAdded<GUIScrollbarComponent>(Entity entity, GUIScrollbarComponent& component)
	{
		GetOrAddComponent<GUIComponent>(entity);
	}
#pragma endregion

#pragma endregion

	bool Scene::EntityExists(Entity e)
	{
		if (!e)
			return false;
		return m_Registry.any_of(e);
	}

	Entity Scene::GetPrimaryCameraEntity(uint64_t layerID)
	{
		Entity retEntity = {};
		std::vector<Entity> entities = GetAllEntitiesWith<CameraComponent>();
		for (Entity entity : entities)
		{
			auto& rc = GetComponent<RenderComponent>(entity);
			auto& cc = GetComponent<CameraComponent>(entity);
			if (cc.Primary && rc.IDHandled(layerID))
			{
				retEntity = entity;
				cc.ActiveCamera.SetViewport(m_Config.ViewportWidth, m_Config.ViewportHeight);
			}
		}
		entities.clear();
		entities = std::vector<Entity>();
		return retEntity;
	}

	Entity Scene::GetGUICanvasEntity(uint64_t layerID)
	{
		Entity retEntity = {};
		std::vector<Entity> entities = GetAllEntitiesWith<GUICanvasComponent>();
		for (Entity entity : entities)
		{
			auto& ac = GetComponent<ActiveComponent>(entity);
			if (ac.Active && !ac.Hidden)
			{
				retEntity = entity;
				break;
			}
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
				const auto& idc = GetComponent<IDComponent>(entity);
				if (uuid == idc.ID)
				{
					retEntity = entity;
					break;
				}
			}
			entities.clear();
			entities = std::vector<Entity>();
		}
		return retEntity;
	}

	Entity Scene::GetEntityByName(const std::string& name)
	{
		Entity retEntity = {};
		if (name.empty())
			return retEntity;

		std::vector<Entity> entities = GetAllEntitiesWith<NameComponent>();
		for (Entity entity : entities)
		{
			const auto& nc = GetComponent<NameComponent>(entity);
			if (name == nc.Name)
			{
				retEntity = entity;
				break;
			}
		}
		entities.clear();
		entities = std::vector<Entity>();
		return retEntity;
	}

	void Scene::GetTotalOffset(Entity entity, glm::vec3& translation, glm::vec3& rotation)
	{
		// Valid Entity
		if (entity && HasComponent<RelationshipComponent>(entity))
		{
			auto& idc = GetComponent<IDComponent>(entity);
			auto& rc = GetComponent<RelationshipComponent>(entity);
			// Entity is a child
			if (rc.Parent != idc.ID)
			{
				auto& trsc = GetComponent<TransformComponent>(entity);
				if (Entity parentEntity = GetEntityByUUID(rc.Parent))
				{
					// Add parent offset
					auto& parentTRSC = GetComponent<TransformComponent>(parentEntity);
					translation += parentTRSC.Translation;
					rotation += parentTRSC.Rotation;
					GetTotalOffset(parentEntity, translation, rotation);
				}
			}
		}
	}

	std::vector<Entity> Scene::GetAllRenderEntities(const uint64_t& layerID)
	{
		std::vector<Entity> renderEntities = std::vector<Entity>();

		std::vector<Entity> entities = GetAllEntitiesWith<RenderComponent>();
		for (Entity entity : entities)
		{
			auto& rc = GetComponent<RenderComponent>(entity);
			for (uint64_t id : rc.LayerIDs)
			{
				if (id == layerID)
				{
					renderEntities.push_back(entity);
				}
			}
		}

		entities.clear();
		entities = std::vector<Entity>();
		return renderEntities;
	}

	void Scene::SetEntityParent(Entity child, const UUID& parentID)
	{
		auto& childIDC = GetOrAddComponent<IDComponent>(child);
		auto& childRSC = GetOrAddComponent<RelationshipComponent>(child);
		if (childRSC.Parent != childIDC.ID) // Parent isn't self, meaning parent needs to know it no longer has this child
		{
			if (Entity parent = GetEntityByUUID(childRSC.Parent))
			{
				auto& parentRSC = GetComponent<RelationshipComponent>(parent);
				parentRSC.RemoveChild(childIDC.ID);
			}
		}
		childRSC.Parent = parentID;
	}

	void Scene::ResetEntityRenderComponents()
	{
		std::vector<Entity> entities = GetAllEntitiesWith<RenderComponent>();
		for (Entity entity : entities)
		{
			GetComponent<RenderComponent>(entity).Rendered = false;
		}
		entities.clear();
		entities = std::vector<Entity>();
	}

	void Scene::SyncCamera(Entity entity, const glm::vec3& position, const glm::vec3& rotation)
	{
		if (HasComponent<CameraComponent>(entity))
		{
			auto& cc = GetComponent<CameraComponent>(entity);
			cc.ActiveCamera.SetViewport(m_Config.ViewportWidth, m_Config.ViewportWidth);
			cc.ActiveCamera.SetPosition(position);
			cc.ActiveCamera.SetRotation(rotation);
		}
	}
	
	Entity Scene::CreateEntity(const std::string& name, uint32_t tagID)
	{
		return CreateEntityWithUUID(UUID(), name, tagID);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, uint32_t tagID)
	{
		entt::entity internalEntity = m_Registry.create();
		Entity entity = Entity((uint32_t)internalEntity);

		AddComponent<IDComponent>(entity, uuid);
		AddComponent<TagComponent>(entity, tagID);
		AddComponent<NameComponent>(entity, name);
		AddComponent<ActiveComponent>(entity);
		AddComponent<RelationshipComponent>(entity, uuid);
		AddComponent<TransformComponent>(entity);
		AddComponent<RenderComponent>(entity, false);
		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		std::string name = GetComponent<NameComponent>(entity).Name;
		uint32_t tagID = GetComponent<TagComponent>(entity).TagID;
		Entity newEntity = CreateEntity(name, tagID);

		CopyComponentIfExists<ActiveComponent>(this, newEntity, entity);
		CopyComponentIfExists<RelationshipComponent>(this, newEntity, entity);
		CopyComponentIfExists<TransformComponent>(this, newEntity, entity);

		CopyComponentIfExists<RenderComponent>(this, newEntity, entity);
		CopyComponentIfExists<CameraComponent>(this, newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(this, newEntity, entity);
		CopyComponentIfExists<CircleRendererComponent>(this, newEntity, entity);
		CopyComponentIfExists<TextRendererComponent>(this, newEntity, entity);

		CopyComponentIfExists<GUIComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUICanvasComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUILayoutComponent>(this, newEntity, entity);
		// TODO : GUIMaskComponent
		CopyComponentIfExists<GUIImageComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUIButtonComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUIInputFieldComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUISliderComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUICheckboxComponent>(this, newEntity, entity);
		// TODO : GUIScrollRectComponent & GUIScrollbarComponent

		CopyComponentIfExists<AudioSourceComponent>(this, newEntity, entity);
		CopyComponentIfExists<AudioListenerComponent>(this, newEntity, entity);

		CopyComponentIfExists<NativeScriptComponent>(this, newEntity, entity);
		CopyComponentIfExists<ScriptComponent>(this, newEntity, entity);

		CopyComponentIfExists<Rigidbody2DComponent>(this, newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(this, newEntity, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(this, newEntity, entity);
		return newEntity;
	}

	Entity Scene::CopyEntity(Entity entity)
	{
		Entity newEntity = CreateEntityWithUUID(GetComponent<IDComponent>(entity).ID,
			GetComponent<NameComponent>(entity).Name, GetComponent<TagComponent>(entity).TagID);

		CopyComponentIfExists<ActiveComponent>(this, newEntity, entity);
		CopyComponentIfExists<RelationshipComponent>(this,newEntity, entity);
		CopyComponentIfExists<TransformComponent>(this,newEntity, entity);

		CopyComponentIfExists<RenderComponent>(this,newEntity, entity);
		CopyComponentIfExists<CameraComponent>(this,newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(this,newEntity, entity);
		CopyComponentIfExists<CircleRendererComponent>(this,newEntity, entity);
		CopyComponentIfExists<TextRendererComponent>(this,newEntity, entity);

		CopyComponentIfExists<GUIComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUICanvasComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUILayoutComponent>(this, newEntity, entity);
		// TODO : GUIMaskComponent
		CopyComponentIfExists<GUIImageComponent>(this, newEntity, entity);
		CopyComponentIfExists<GUIButtonComponent>(this,newEntity, entity);
		CopyComponentIfExists<GUIInputFieldComponent>(this,newEntity, entity);
		CopyComponentIfExists<GUISliderComponent>(this,newEntity, entity);
		CopyComponentIfExists<GUICheckboxComponent>(this, newEntity, entity);
		// TODO : GUIScrollRectComponent & GUIScrollbarComponent

		CopyComponentIfExists<AudioSourceComponent>(this,newEntity, entity);
		CopyComponentIfExists<AudioListenerComponent>(this,newEntity, entity);

		CopyComponentIfExists<NativeScriptComponent>(this,newEntity, entity);
		CopyComponentIfExists<ScriptComponent>(this,newEntity, entity);

		CopyComponentIfExists<Rigidbody2DComponent>(this,newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(this,newEntity, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(this,newEntity, entity);
		return newEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		auto& idc = GetComponent<IDComponent>(entity);
		auto& rsc = GetComponent<RelationshipComponent>(entity);

		// Clear self from parent, if parent != self
		if (rsc.Parent != idc.ID)
		{
			if (Entity parent = GetEntityByUUID(rsc.Parent))
			{
				auto& parentRSC = GetComponent<RelationshipComponent>(parent);
				parentRSC.RemoveChild(idc.ID);
			}
		}

		// Clear all children from registry
		for (const UUID& childID : rsc.Children)
		{
			if (Entity child = GetEntityByUUID(childID))
			{
				// Setting childs parent == childs/self 
				// because it doesn't need to remove itself from its parent, in this case.
				auto& childRSC = GetComponent<RelationshipComponent>(child);
				childRSC.Parent = childID;
				DestroyEntity(child);
			}
		}
		m_Registry.destroy(entity);
	}

#pragma endregion

#pragma endregion
}