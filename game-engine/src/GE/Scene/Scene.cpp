#include "GE/GEpch.h"

#include "Scene.h"

#include "GE/Scene/Components/Components.h"
#include "GE/Scene/Entity/ScriptableEntity.h"
#include "GE/Scripting/Scripting.h"

#include "GE/Rendering/Renderer/2D/Renderer2D.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace GE
{
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
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity)
	{

	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity)
	{
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity)
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

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity)
	{
	}

#pragma endregion

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

	Scene::Scene()
	{

	}

	Scene::Scene(const std::string& name) : m_Name(name)
	{

	}

	Scene::~Scene()
	{

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
			GE_PROFILE_SCOPE("Scene - Copy : Components");

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
			CopyComponent<CircleRendererComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CameraComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<NativeScriptComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<Rigidbody2DComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<BoxCollider2DComponent>(newSceneRegistry, sceneRegistry, entityMap);
			CopyComponent<CircleCollider2DComponent>(newSceneRegistry, sceneRegistry, entityMap);
		}

		return newScene;
	}

	void Scene::Render(const EditorCamera& camera)
	{
		if (&camera)
		{
			GE_PROFILE_SCOPE("Scene::Render");
			Renderer2D::Start(camera);

			// Sprites
			{
				auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
				for (auto entity : view)
				{
					auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				}
			}

			// Circles
			{
				auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
				for (auto entity : view)
				{
					auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::FillCircle(transform.GetTransform(), circle.Color, circle.Radius, circle.Thickness, circle.Fade, (int)entity);

				}
			}

			// Physics Visuals
			{

				// Renders Entity Box Colliders
				{
					auto view = m_Registry.view<TransformComponent, BoxCollider2DComponent>();
					for (auto e : view)
					{
						auto [tc, bc2D] = view.get<TransformComponent, BoxCollider2DComponent>(e);
						if (bc2D.Show)
						{
							glm::vec3 translation = tc.Translation + glm::vec3(bc2D.Offset, 0.0025f);
							glm::vec3 scale = tc.Scale * glm::vec3(bc2D.Size * 2.0f, 1.0f);
							glm::mat4 transform = glm::translate(Renderer2D::s_IdentityMat4, translation)
								* glm::rotate(Renderer2D::s_IdentityMat4, glm::radians(tc.Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f))
								* glm::scale(Renderer2D::s_IdentityMat4, scale);
							Renderer2D::DrawRectangle(transform, glm::vec4(0, 1, 0, 1));
						}
					}
				}

				// Renders Entity Circle Colliders
				{
					auto view = m_Registry.view<TransformComponent, CircleCollider2DComponent>();
					for (auto e : view)
					{
						auto [tc, cc2D] = view.get<TransformComponent, CircleCollider2DComponent>(e);
						if (cc2D.Show)
						{
							glm::vec3 translation = tc.Translation + glm::vec3(cc2D.Offset, 0.0025f);
							glm::vec3 scale = tc.Scale * glm::vec3(glm::vec2(cc2D.Radius * 2.0f), 1.0f);
							glm::mat4 transform = glm::translate(Renderer2D::s_IdentityMat4, translation)
								* glm::scale(Renderer2D::s_IdentityMat4, scale);
							Renderer2D::FillCircle(transform, glm::vec4(0, 1, 0, 1), cc2D.Radius, 0.5f, 0.4f);
						}
					}
				}

			}

			Renderer2D::End();

		}
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
	
	Entity Scene::GetEntity(UUID uuid)
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
		CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(newEntity, entity);

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
	
	void Scene::OnStop()
	{
		DestroyPhysics2D();
		DestroyScripting();

		m_SceneState = Scene::SceneState::Stop;
	}

	void Scene::OnRuntimeStart()
	{
		m_SceneState = Scene::SceneState::Run;

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
		{
			GE_PROFILE_SCOPE("Scene::OnRuntimeUpdate -- Camera & 2D Renderer");
			Camera* mainCamera = nullptr;
			glm::mat4* cameraTransform;

			// Finds & Updates Primary Camera Transform
			{
				GE_PROFILE_SCOPE("Scene::OnRuntimeUpdate -- Camera");

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
				GE_PROFILE_SCOPE("Scene::OnRuntimeUpdate -- 2D Renderer");

				Renderer2D::Start(*mainCamera, *cameraTransform);

				// Sprites
				{
					auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
					for (auto entity : view)
					{
						auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);

						Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
					}
				}

				// Circles
				{
					auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
					for (auto entity : view)
					{
						auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

						Renderer2D::FillCircle(transform.GetTransform(), circle.Color, circle.Radius, circle.Thickness, circle.Fade, (int)entity);
					}
				}

				Renderer2D::End();
			}
		}
	}

	void Scene::OnSimulationStart()
	{
		m_SceneState = Scene::SceneState::Simulate;
		
		InitializePhysics2D();
		InitializeScripting();
	}

	void Scene::OnSimulationUpdate(Timestep timestep, EditorCamera& camera)
	{
		UpdatePhysics2D(timestep);
		UpdateScripting(timestep);
		Render(camera);
	}

	void Scene::OnEditorUpdate(Timestep timestep, EditorCamera& camera)
	{
		Render(camera);
	}

#pragma region Physics
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

	void Scene::InitializePhysics2D()
	{

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

			//	Box Collider
			{
				GE_PROFILE_SCOPE("Scene - InitializePhysics2D : Box Collider");
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

			//	Circle Collider
			{
				GE_PROFILE_SCOPE("Scene - InitializePhysics2D : Circle Collider");
				if (entity.HasComponent<CircleCollider2DComponent>())
				{
					auto& cc2D = entity.GetComponent<CircleCollider2DComponent>();
					auto& tc = entity.GetComponent<TransformComponent>();

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

	void Scene::DestroyPhysics2D()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}
#pragma endregion

#pragma region Scripting
	void Scene::InitializeScripting()
	{
		GE_PROFILE_SCOPE("Scene::InitializeScripting");
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = { entity, this };
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
		GE_PROFILE_SCOPE("Scene::UpdateScripting");
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
		GE_PROFILE_SCOPE("Scene::DestroyScripting");
		Scripting::OnStop();
	}
#pragma endregion

}