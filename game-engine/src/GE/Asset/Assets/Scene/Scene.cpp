#include "GE/GEpch.h"

#include "Scene.h"
#include "Components/Components.h"

#include "GE/Physics/Physics.h"
#include "GE/Project/Project.h"

#include "GE/Rendering/Renderer/2D/Renderer2D.h"

#include "GE/Scripting/Scripting.h"

namespace GE
{
	template<typename Component>
	static void CopyComponent(entt::registry& to, entt::registry& from, const std::unordered_map<UUID, entt::entity>& entityMap)
	{
		auto view = from.view<Component>();
		for (auto e : view)
		{
			UUID uuid = from.get<IDComponent>(e).ID;
			GE_CORE_ASSERT(entityMap.find(uuid) != entityMap.end(), "Could not find Entity in map.");
			entt::entity enttID = entityMap.at(uuid);
			auto& component = from.get<Component>(e);
			to.emplace_or_replace<Component>(enttID, component);
		}
	}

	template<typename Component>
	static void CopyComponentIfExists(Entity to, Entity from)
	{
		if (from.HasComponent<Component>())
		{
			Component component = from.GetComponent<Component>();
			to.AddOrReplaceComponent<Component>(component);
		}
	}

#pragma region Entity
	Entity::Entity(entt::entity entityID, Scene* scene) : m_EntityID(entityID), m_Scene(scene)
	{
		if (m_Scene == nullptr)
			m_Scene = Project::GetAsset<Scene>(Project::GetConfig().SceneHandle).get();
	}
	
	Entity::~Entity()
	{
		m_Scene = nullptr;
		ClearEntityID();
	}

#pragma region OnComponentAdded

	template<typename T>
	void Entity::OnComponentAdded()
	{
		static_assert(false);
	}

	template<>
	void Entity::OnComponentAdded<IDComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<TagComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<TransformComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<CameraComponent>()
	{
		this->GetComponent<CameraComponent>().ActiveCamera.SetViewport(this->m_Scene->GetConfig().ViewportWidth, this->m_Scene->GetConfig().ViewportWidth);
	}

	template<>
	void Entity::OnComponentAdded<AudioSourceComponent>()
	{
		
	}

	template<>
	void Entity::OnComponentAdded<AudioListenerComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<SpriteRendererComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<CircleRendererComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<TextRendererComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<NativeScriptComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<ScriptComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<Rigidbody2DComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<BoxCollider2DComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<CircleCollider2DComponent>()
	{
	}
#pragma endregion

#pragma endregion

#pragma region Scene

	Scene::Scene() : Asset(UUID(), Asset::Type::Scene)
	{

	}

	Scene::Scene(UUID handle, const std::string& name) : Asset(handle, Scene::Type::Scene)
	{
		m_Config.Name = name;
	}

	Scene::~Scene()
	{
		m_Registry.clear();
	}

	Ref<Asset> Scene::GetCopy()
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->p_Handle = this->p_Handle;
		newScene->m_Config.ViewportWidth = this->m_Config.ViewportWidth;
		newScene->m_Config.ViewportHeight = this->m_Config.ViewportHeight;
		newScene->m_Config.State = this->m_Config.State;
		newScene->m_Config.Name = this->m_Config.Name;

		auto& sceneRegistry = this->m_Registry;
		auto& newSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> entityMap;

		// Copy Components
		{
			GE_PROFILE_SCOPE("Scene - Copy : Components");

			// Special Case - Find/Set ID & Tag
			auto idView = sceneRegistry.view<IDComponent>();
			for (auto e : idView)
			{
				UUID uuid = sceneRegistry.get<IDComponent>(e).ID;
				const auto& name = sceneRegistry.get<TagComponent>(e).Tag;
				Entity entity = newScene->CreateEntityWithUUID(uuid, name);
				entityMap.emplace(uuid, entity);
			}

			CopyComponent<TransformComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CameraComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<AudioSourceComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<AudioListenerComponent>(newSceneRegistry, sceneRegistry, entityMap);
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

#pragma region Entity Control
	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity(entity, this);
		}
		return { };
	}
	
	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			const auto& idc = view.get<IDComponent>(entity);
			if (uuid == idc.ID)
				return Entity(entity, this);
		}
		return { };
	}

	Entity Scene::GetEntityByTag(const std::string& tag)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const auto& nc = view.get<TagComponent>(entity);
			if (tag == nc.Tag)
				return Entity(entity, this);
		}
		return Entity{ };
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& tag)
	{
		Entity entity = { m_Registry.create(), this };

		AddComponent<IDComponent>(entity, uuid);
		AddComponent<TransformComponent>(entity);

		auto& name = AddComponent<TagComponent>(entity);
		name.Tag = tag.empty() ? "Entity" : tag;

		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetComponent<TagComponent>().Tag;
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
		CopyComponentIfExists<TextRendererComponent>(newEntity, entity);
		CopyComponentIfExists<AudioSourceComponent>(newEntity, entity);
		CopyComponentIfExists<AudioListenerComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
		CopyComponentIfExists<ScriptComponent>(newEntity, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(newEntity, entity);

		return newEntity;
	}

	Entity Scene::CopyEntity(Entity entity)
	{
		std::string name = entity.GetComponent<TagComponent>().Tag;
		Entity newEntity = CreateEntityWithUUID(entity.GetComponent<IDComponent>().ID, name);

		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
		CopyComponentIfExists<TextRendererComponent>(newEntity, entity);
		CopyComponentIfExists<AudioSourceComponent>(newEntity, entity);
		CopyComponentIfExists<AudioListenerComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
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

	void Scene::OnResizeViewport(uint32_t width, uint32_t height)
	{
		if (m_Config.ViewportWidth == width && m_Config.ViewportHeight == height)
			return;

		if (width == 0 || height == 0)
		{
			GE_CORE_WARN("Scene failed to resize viewport\n\tWidth: {0}\n\tHeight: {1}", width, height);
			return;
		}

		m_Config.ViewportWidth = width;
		m_Config.ViewportHeight = height;

		// Resize non-fixed aspect ratio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			if (!camera.FixedAspectRatio)
			{
				auto& c = camera.ActiveCamera;
				const glm::vec2 vp = c.GetViewport();
				if (vp.x != width && vp.y != height)
					c.SetViewport(width, height);
			}
		}
	}

	void Scene::OnStart(State state, uint32_t viewportWidth, uint32_t viewportHeight)
	{
		OnResizeViewport(viewportWidth, viewportHeight);

		switch (state)
		{
		case State::Run:
			OnRuntimeStart();
			break;
		case State::Simulate:
			OnSimulationStart();
			break;
		case State::Pause:
			OnPauseStart();
			break;
		default:
			GE_CORE_INFO("Cannot Start Scene.\n\tState: {0}", SceneUtils::SceneStateToString(state));
			break;
		}
	}

	void Scene::OnUpdate(Timestep ts, Camera* camera)
	{
		switch (m_Config.State)
		{
		case State::Stop:
			OnEditorUpdate(ts, camera);
			break;
		case State::Run:
			OnRuntimeUpdate(ts);
			break;
		case State::Simulate:
			OnSimulationUpdate(ts, camera);
			break;
		case State::Pause:
			OnPauseUpdate(ts, camera);
			break;
		default:
			GE_ASSERT(false, "Unsupported Scene State.");
			break;
		}
	}

	void Scene::OnStop()
	{
		m_Config.State = State::Stop;
		
		DestroyPhysics2D();
		DestroyScripting();
	}

	void Scene::OnRuntimeStart()
	{
		m_Config.State = State::Run;

		InitializePhysics2D();
		InitializeScripting();
	}
	
	void Scene::OnRuntimeUpdate(Timestep timestep)
	{
		// Update Physics
		UpdatePhysics2D(timestep);

		// Update Scripts
		UpdateScripting(timestep);

		// Update Camera & 2D Renderer
		Render();
	}

	void Scene::OnSimulationStart()
	{
		m_Config.State = State::Simulate;
		
		InitializePhysics2D();
		InitializeScripting();
	}

	void Scene::OnSimulationUpdate(Timestep timestep, Camera* camera)
	{
		UpdatePhysics2D(timestep);
		UpdateScripting(timestep);

		if (camera)
		{
			camera->OnUpdate(timestep);
			Render(camera);
		}
		else
		{
			Render();
		}
	}

	void Scene::OnPauseStart()
	{
		m_Config.State = State::Pause;

	}

	void Scene::OnPauseUpdate(Timestep timestep, Camera* camera)
	{	
		if (m_Config.StepFrames > 0)
		{
			UpdatePhysics2D(timestep);
			UpdateScripting(timestep);
			m_Config.StepFrames--;
		}

		if (camera)
		{
			camera->OnUpdate(timestep);
			Render(camera);
		}
		else
		{
			Render();
		}
	}

	void Scene::OnStep(int steps)
	{
		m_Config.StepFrames += steps;
	}

	void Scene::OnEditorUpdate(Timestep timestep, Camera* camera)
	{
		if (camera)
		{
			camera->OnUpdate(timestep);
			Render(camera);
		}
		else
		{
			Render();
		}
	}

	void Scene::Render()
	{
		Camera* camera = nullptr;

		// Find Primary Camera
		auto view = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : view)
		{
			auto [tc, cc] = view.get<TransformComponent, CameraComponent>(entity);
			if (cc.Primary)
			{
				camera = &cc.ActiveCamera;
				camera->SetViewMatrix(tc.GetTransform());
				break;
			}
		}

		Render(camera);
	}

	void Scene::Render(Camera* camera)
	{
		if (&camera)
		{
			GE_PROFILE_SCOPE("Scene -- Render");

			glm::vec2 viewport = camera->GetViewport();
			OnResizeViewport((uint32_t)viewport.x, (uint32_t)viewport.y);

			Renderer2D::Start(*camera);

			// Sprites
			auto spriteView = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto entity : spriteView)
			{
				auto [transform, sprite] = spriteView.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}

			// Circles
			auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : circleView)
			{
				auto [transform, circle] = circleView.get<TransformComponent, CircleRendererComponent>(entity);

				Renderer2D::DrawCircle(transform.GetTransform(), circle, (int)entity);
			}

			// Text
			auto textView = m_Registry.view<TransformComponent, TextRendererComponent>();
			for (auto entity : textView)
			{
				auto [transform, text] = textView.get<TransformComponent, TextRendererComponent>(entity);

				Renderer2D::DrawString(transform.GetTransform(), text, (int)entity);
			}

			// Box Colliders
			auto bcView = m_Registry.view<TransformComponent, BoxCollider2DComponent>();
			for (auto entity : bcView)
			{
				auto [tc, bc2D] = bcView.get<TransformComponent, BoxCollider2DComponent>(entity);
				if (bc2D.Show)
				{
					/*glm::vec3 translation = tc.Translation + glm::vec3(bc2D.Offset, 0.0025f);
					glm::vec3 scale = tc.Scale * glm::vec3(bc2D.Size * 2.0f, 1.0f);
					glm::mat4 transform = glm::translate(Renderer2D::GetIdentityMat4(), translation)
						* glm::rotate(Renderer2D::GetIdentityMat4(), glm::radians(tc.Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f))
						* glm::scale(Renderer2D::GetIdentityMat4(), scale);*/
					
					Renderer2D::DrawRectangle(tc.GetTransform(glm::vec3(bc2D.Offset, 0.0025f), glm::vec3(bc2D.Size * 2.0f, 1.0f)), glm::vec4(0, 1, 0, 1), (int)entity);
				}
			}

			// Circle Colliders
			auto ccView = m_Registry.view<TransformComponent, CircleCollider2DComponent>();
			for (auto entity : ccView)
			{
				auto [tc, cc2D] = ccView.get<TransformComponent, CircleCollider2DComponent>(entity);
				if (cc2D.Show)
				{
					/*glm::vec3 translation = tc.Translation + glm::vec3(cc2D.Offset, 0.0025f);
					glm::vec3 scale = tc.Scale * glm::vec3(glm::vec2(cc2D.Radius) * 4.0f, 1.0f);
					glm::mat4 transform = glm::translate(Renderer2D::GetIdentityMat4(), translation)
						* glm::scale(Renderer2D::GetIdentityMat4(), scale);*/
					Renderer2D::DrawCircle(tc.GetTransform(glm::vec3(cc2D.Offset, 0.0025f), glm::vec3(glm::vec2(cc2D.Radius) * 4.0f, 1.0f)), glm::vec4(0, 1, 0, 1), cc2D.Radius, 0.5f, 0.4f, (int)entity);
				}
			}

			Renderer2D::End();

		}
	}

#pragma region Physics

	void Scene::InitializePhysics2D()
	{
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
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
	}
	
	void Scene::UpdatePhysics2D(Timestep timestep)
	{
		GE_PROFILE_SCOPE("Scene - UpdatePhysics2D");
		if (m_PhysicsWorld)
		{
			const int32_t velocityInteration = 5;
			const int32_t positionInteration = 5;
			m_PhysicsWorld->Step(timestep, velocityInteration, positionInteration);

			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transformComponent = GetComponent<TransformComponent>(entity);
				auto& rb2D = GetComponent<Rigidbody2DComponent>(entity);

				GE_CORE_ASSERT(rb2D.RuntimeBody != nullptr, "Rigidbody2DComponent has no Runtime Body.");
				b2Body* body = (b2Body*)rb2D.RuntimeBody;
				const auto& position = body->GetPosition();
				transformComponent.Translation.x = position.x;
				transformComponent.Translation.y = position.y;

				transformComponent.Rotation.z = body->GetAngle();
			}
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
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->SetID((uint32_t)entity);
					nsc.Instance->OnCreate();
				}

			});

		Scripting::SetScene(this);

		m_Registry.view<ScriptComponent>().each([=](auto entity, auto& sc)
			{
				Entity e = { entity, this };
				Scripting::OnCreateScript(e);
			});
	}

	void Scene::UpdateScripting(Timestep timestep)
	{
		GE_PROFILE_SCOPE("UpdateScripting");
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				nsc.Instance->OnUpdate(timestep);
			});

		m_Registry.view<ScriptComponent>().each([=](auto entity, auto& sc)
			{
				Entity e = { entity, this };
				Scripting::OnUpdateScript(e, timestep);
			});
	}

	void Scene::DestroyScripting()
	{
		GE_PROFILE_SCOPE("DestroyScripting");
		Scripting::OnStop();
	}
#pragma endregion

#pragma endregion
}