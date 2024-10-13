#include "GE/GEpch.h"

#include "Layer.h"

#include "GE/Core/Application/Application.h"
#include "GE/Core/Input/Input.h"

#include "GE/Project/Project.h"
#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	Layer::Layer(uint64_t id, bool isBase)
	{
		GE_PROFILE_FUNCTION();
		p_Config = Config(id, isBase);
	}

	Layer::~Layer()
	{
		GE_PROFILE_FUNCTION();
	}

	bool Layer::Validate(Entity entity) const
	{
		if (entity && entity.HasComponent<RenderComponent>())
		{
			// Validate that layer should handle Entity
			RenderComponent rc = entity.GetComponent<RenderComponent>();
			for (uint64_t id : rc.LayerIDs)
			{
				if (id == p_Config.ID)
				{
					// Entity validated
					return true;
				}
			}
		}
		return false;
	}

	void Layer::OnAttach()
	{
		GE_PROFILE_FUNCTION();

	}

	void Layer::OnDetach()
	{
		GE_PROFILE_FUNCTION();

	}

	void Layer::OnUpdate(Timestep ts)
	{
		GE_PROFILE_FUNCTION();

		Ref<Scene> runtimeScene = Project::GetRuntimeScene();
		if (runtimeScene && !runtimeScene->IsStopped())
		{
			if (Entity entity = runtimeScene->GetPrimaryCameraEntity(p_Config.ID))
			{
				Camera* camera = &entity.GetComponent<CameraComponent>().ActiveCamera;
				OnRender(camera);
			}
		}
	}

	void Layer::OnRender(Camera* camera)
	{
		GE_PROFILE_SCOPE("Layer::OnRender()");

		if (Ref<Scene> runtimeScene = Project::GetRuntimeScene())
		{
			if (camera)
			{
				Renderer::Open(*camera);

				std::vector<Entity> entities = runtimeScene->GetAllEntitiesWith<RenderComponent>();
				for (Entity entity : entities)
				{
					auto& rc = entity.GetComponent<RenderComponent>();
					if (rc.Rendered || !Validate(entity))
						continue;
					rc.Rendered = true;

					auto& trsc = entity.GetComponent<TransformComponent>();

					if (entity.HasComponent<SpriteRendererComponent>())
					{
						auto& src = entity.GetComponent<SpriteRendererComponent>();
						Renderer::DrawTexture(trsc.GetTransform(), src, entity);
					}

					if (entity.HasComponent<CircleRendererComponent>())
					{
						auto& crc = entity.GetComponent<CircleRendererComponent>();
						Renderer::DrawSphere(trsc.GetTransform(), crc, entity);
					}

					if (entity.HasComponent<TextRendererComponent>())
					{
						auto& trc = entity.GetComponent<TextRendererComponent>();
						Renderer::DrawTxt(trsc.GetTransform(), trc, entity);
					}
			
				}
				entities.clear();
				entities = std::vector<Entity>();

				Renderer::Close();
			}
		}
	}

	void Layer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_EVENT_FN(OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(GE_BIND_EVENT_FN(OnMousePressed));
	}

	bool Layer::OnKeyPressed(KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
		case Input::KEY_0:
			break;
		default:
			break;
		}
		return false;
	}

	bool Layer::OnMousePressed(MouseButtonPressedEvent& e)
	{
		switch (e.GetMouseButton())
		{
		case Input::MouseCode::MOUSE_BUTTON_1:
		{
			Entity hoveredEntity = Application::GetHoveredEntity();
			if(Validate(hoveredEntity))
				return Project::SceneEvent(e, hoveredEntity);
		}
			break;
		default:
			break;
		}
		return false;
	}

}