#include "GE/GEpch.h"
#include "Scene.h"
#include "Components/Components.h"
#include "Entity/Entity.h"
#include "GE/Rendering/Renderer/2D/Renderer2D.h"

namespace GE
{
#pragma region OnComponentAdded

	template<typename T>
	void Scene::OnComponentAdded(Entity entity)
	{
		static_assert(false);
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

#pragma endregion

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
			
		return entity;
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

	void Scene::OnUpdate(Timestep timestep)
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
}