#include "GE/GEpch.h"

#include "Layer.h"

#include "GE/Asset/Assets/Scene/Entity.h"
#include "GE/Asset/Assets/Scene/Scene.h"

#include "GE/Core/Application/Application.h"
#include "GE/Core/Input/Input.h"

#include "GE/Project/Project.h"
#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	Layer::Layer(uint64_t id)
	{
		GE_PROFILE_FUNCTION();
		p_Config = Config(id);
	}

	Layer::~Layer()
	{
		GE_PROFILE_FUNCTION();
	}

	void Layer::RenderEntity(Ref<Scene> scene, Entity entity, glm::vec3& translationOffset, glm::vec3& rotationOffset)
	{
		GE_PROFILE_FUNCTION();
		if (!scene || !entity)
			return; 

		auto& idc = scene->GetComponent<IDComponent>(entity);
		auto& ac = scene->GetComponent<ActiveComponent>(entity);
		auto& rc = scene->GetComponent<RenderComponent>(entity);
		if (rc.Rendered || (!ac.Active || ac.Hidden) || !rc.IDHandled(p_Config.ID))
			return; // Entity; 1: Shouldn't be rendered. 2: Should be active && visible. 3: Should handle current LayerID

		auto& trsc = scene->GetComponent<TransformComponent>(entity);

		// First, render self/parent
		if (scene->HasComponent<SpriteRendererComponent>(entity))
		{
			auto& src = scene->GetComponent<SpriteRendererComponent>(entity);
			Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), src, entity);
			rc.Rendered = true;
		}

		if (scene->HasComponent<CircleRendererComponent>(entity))
		{
			auto& crc = scene->GetComponent<CircleRendererComponent>(entity);
			Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), crc, entity);
			rc.Rendered = true;
		}

		if (scene->HasComponent<TextRendererComponent>(entity))
		{
			auto& trc = scene->GetComponent<TextRendererComponent>(entity);
			Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trc, entity);
			rc.Rendered = true;
		}

		// Then, render children offset from self/parent
		auto& rsc = scene->GetComponent<RelationshipComponent>(entity);
		if (!rsc.GetChildren().empty())
		{
			translationOffset += trsc.Translation;
			rotationOffset += trsc.Rotation;
			for (const UUID& childID : rsc.GetChildren())
			{
				Entity childEntity = scene->GetEntityByUUID(childID);
				RenderEntity(scene, childEntity, translationOffset, rotationOffset);
			}
		}

	}

	void Layer::OnAttach(Ref<Scene> scene)
	{
		GE_PROFILE_FUNCTION();

	}

	void Layer::OnDetach()
	{
		GE_PROFILE_FUNCTION();

	}

	void Layer::OnUpdate(Ref<Scene> scene, Timestep ts)
	{
		GE_PROFILE_FUNCTION();

		if (scene && !scene->IsStopped())
		{
			if (Entity entity = scene->GetPrimaryCameraEntity(p_Config.ID))
			{
				auto& cc = scene->GetComponent<CameraComponent>(entity);
				cc.OnUpdate(ts);
				const Camera* camera = &cc.ActiveCamera;
				OnRender(scene, camera);
			}
		}
	}

	void Layer::OnRender(Ref<Scene> scene, const Camera*& camera)
	{
		GE_PROFILE_FUNCTION();

		if (scene && camera)
		{
			Renderer::Open(camera);

			std::vector<Entity> entities = scene->GetAllRenderEntities(p_Config.ID);
			for (Entity entity : entities)
			{
				auto& idc = scene->GetComponent<IDComponent>(entity);
				auto& rsc = scene->GetComponent<RelationshipComponent>(entity);
				if (idc.ID == rsc.GetParent()) // Is Parent
				{
					Layer::RenderEntity(scene, entity, glm::vec3(0.0f), glm::vec3(0.0f));
				}
			}

			entities.clear();
			entities = std::vector<Entity>();

			Renderer::Close();
		}
	}

	void Layer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_EVENT_FN(OnKeyPressed));
		dispatcher.Dispatch<MousePressedEvent>(GE_BIND_EVENT_FN(OnMousePressed));
		dispatcher.Dispatch<MouseMovedEvent>(GE_BIND_EVENT_FN(OnMouseMoved));
		dispatcher.Dispatch<MouseScrolledEvent>(GE_BIND_EVENT_FN(OnMouseScrolled));

	}

	bool Layer::OnKeyPressed(KeyPressedEvent& e)
	{
		return false;
	}

	bool Layer::OnMousePressed(MousePressedEvent& e)
	{		
		return false;
	}
	bool Layer::OnMouseMoved(MouseMovedEvent& e)
	{
		return false;
	}

	bool Layer::OnMouseScrolled(MouseScrolledEvent& e)
	{
		return false;
	}

}