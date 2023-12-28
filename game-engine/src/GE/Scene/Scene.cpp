#include "GE/GEpch.h"
#include "Scene.h"

#include "Entity/ScriptableEntity.h"
#include "GE/Rendering/Renderer/2D/Renderer2D.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_fixture.h>

namespace GE
{
#pragma region Physics Utils
	static b2BodyType GetBox2DType(Rigidbody2DComponent::BodyType type)
	{
		switch (type)
		{
		case GE::Rigidbody2DComponent::BodyType::Static:
			return b2_staticBody;
			break;
		case GE::Rigidbody2DComponent::BodyType::Dynamic:
			return b2_dynamicBody;
			break;
		case GE::Rigidbody2DComponent::BodyType::Kinematic:
			return b2_kinematicBody;
			break;
		}
		GE_CORE_ASSERT(false, "Unsupported Rigidbody2D type.");
		return b2_staticBody;
	}
#pragma endregion


	Scene::Scene()
	{

	}

	Scene::~Scene()
	{

	}

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

	Ref<Scene> Scene::Copy(const Ref<Scene> scene)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = scene->m_ViewportWidth;
		newScene->m_ViewportHeight = scene->m_ViewportHeight;
		newScene->m_SceneState = scene->m_SceneState;

		auto& sceneRegistry = scene->m_Registry;
		auto& newSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> entityMap;

		// Copy Components
		{
			GE_PROFILE_SCOPE();

			// Special Case - Find/Set ID & Tag
			auto idView = sceneRegistry.view<IDComponent>();
			for (auto e : idView)
			{
				UUID uuid = sceneRegistry.get<IDComponent>(e).ID;
				const auto& name = sceneRegistry.get<TagComponent>(e).Tag;
				Entity entity = newScene->CreateEntityWithUUID(uuid, name);
				entityMap[uuid] = (entt::entity)entity;
			}

			CopyComponent<TransformComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<SpriteRendererComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CameraComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<NativeScriptComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<Rigidbody2DComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<BoxCollider2DComponent>(newSceneRegistry, sceneRegistry, entityMap);
		}

		return newScene;
	}

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

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);

		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);

		return newEntity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity.GetEntityID());
	}

	void Scene::ResizeViewport(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize non-fixed aspect ratio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			if (!camera.FixedAspectRatio)
			{
				camera.Camera.SetViewport(width, height);
			}
		}
	}

	void Scene::OnRuntimeStart()
	{
		m_SceneState = Scene::SceneState::Play;

		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = GetBox2DType(rb2D.Type);
			bodyDef.position.Set(transformComponent.Translation.x, transformComponent.Translation.y);
			bodyDef.angle = transformComponent.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2D.FixedRotation);

			rb2D.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape polygonShape;
				polygonShape.SetAsBox(bc2D.Size.x * transformComponent.Scale.x,
					bc2D.Size.y * transformComponent.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &polygonShape;
				fixtureDef.density = bc2D.Density;
				fixtureDef.friction = bc2D.Friction;
				fixtureDef.restitution = bc2D.Restitution;
				fixtureDef.restitutionThreshold = bc2D.RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		m_SceneState = Scene::SceneState::Stop;

		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::OnUpdateRuntime(Timestep timestep)
	{
		// Update Scripts
		{
			GE_PROFILE_SCOPE("Scene::OnUpdate -- Scripts");
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) 
				{
					// TO DO: Move to Scene::Start()
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = { entity, this };
						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(timestep);
				});
		}

		// Update Physics
		{
			const int32_t velocityInteration = 5;
			const int32_t positionInteration = 5;
			m_PhysicsWorld->Step(timestep, velocityInteration, positionInteration);

			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transformComponent = entity.GetComponent<TransformComponent>();
				auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();

				GE_CORE_ASSERT(rb2D.RuntimeBody != nullptr, "Rigidbody2DComponent has no Runtime Body.");
				b2Body* body = (b2Body*)rb2D.RuntimeBody;
				const auto& position = body->GetPosition();
				transformComponent.Translation.x = position.x;
				transformComponent.Translation.y = position.y;

				transformComponent.Rotation.z = body->GetAngle();
			}
		}

		// Update 2D Renderer
		{
			GE_PROFILE_SCOPE("Scene::OnUpdate -- 2D Renderer");
			Camera* mainCamera = nullptr;
			glm::mat4* cameraTransform;
			{
				auto view = m_Registry.view<TransformComponent, CameraComponent>();
				for (auto entity : view)
				{
					auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
					if (camera.Primary)
					{
						mainCamera = &camera.Camera;
						cameraTransform = &transform.GetTransform();
						break;
					}
				}
			}

			if (mainCamera)
			{
				Renderer2D::Start(*mainCamera, *cameraTransform);

				auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
				for (auto entity : view)
				{
					auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::FillQuad(transform.GetTransform(), sprite.Color);
				}

				Renderer2D::End();
			}
		}

	}

	void Scene::OnUpdateEditor(Timestep timestep, EditorCamera& camera)
	{
		{
			GE_PROFILE_SCOPE();
			if (&camera)
			{
				Renderer2D::Start(camera);

				auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
				for (auto entity : view)
				{
					auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				}

				Renderer2D::End();

			}
		}
	}

#pragma region OnComponentAdded

	template<typename T>
	void Scene::OnComponentAdded(Entity entity)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity)
	{

	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity)
	{
		entity.GetComponent<CameraComponent>().Camera.SetViewport(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity)
	{

	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity)
	{
	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity)
	{
	}

#pragma endregion

}