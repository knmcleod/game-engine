#pragma once
#include "Layer.h"

#include "GE/Asset/Assets/Scene/Components/Components.h"
#include "GE/Asset/Assets/Scene/Entity.h"
#include "GE/Core/UUID/UUID.h"

namespace GE
{
	class GUILayer : public Layer
	{
	public:
		struct NavData
		{
			bool IsLayout = false;
			bool IsVertical = false;

			Entity HoveredEntity;
			Entity FocusedEntity;
			Entity SelectedEntity;
		};

		GUILayer() = default;
		GUILayer(uint64_t id);
		~GUILayer() override;

	private:
		/*
		* Renders only Button, InputField, Slider Components on Entity
		* 
		* @param scene : runtime scene
		* @param entity : entity with GUI Components
		* @param translationOffset : offset from parent Entity. Default Worldspace(0.0, 0.0)
		*/
		void RenderEntity(Ref<Scene> scene, Entity entity, glm::vec3& translationOffset, glm::vec3& rotationOffset) override;
		
		void OnAttach(Ref<Scene> scene) override;
		void OnUpdate(Ref<Scene> scene, Timestep ts) override;
		/*
		* Finds GUICanvasComponents in Scene & renders its children using RenderEntity(Ref<Scene>, Entity, const glm::vec3&)
		* 
		* @param scene : runtime scene
		* @param camera : runtime/CameraComponent SceneCamera expected
		*/
		void OnRender(Ref<Scene> scene, const Camera*& camera) override;

		/*
		*
		* @param e : Key Event to handle
		* @param scene : runtime Scene
		* @param entity : root entity to traverse
		*/
		void TraverseGUIEntity(KeyPressedEvent& e, Ref<Scene> scene, const Entity& entity);

		bool OnKeyPressed(KeyPressedEvent& e) override;
		bool OnMousePressed(MousePressedEvent& e) override;
		bool OnMouseScrolled(MouseScrolledEvent& e) override;

		void Hover(Ref<Scene> scene, Entity entity, bool unhoverOld = true);
		void Unhover(Ref<Scene> scene);

		void Focus(Ref<Scene> scene, Entity entity, bool unfocusOld = true);
		void Unfocus(Ref<Scene> scene);

		void Select(Ref<Scene> scene, Entity entity, bool unselectOld = true);
		void Unselect(Ref<Scene> scene);

		// TODO : 
		// Activate(Ref<Scene> scene, Entity entity, bool unactivateOld = true);
		// Unactivate(Ref<Scene> scene);
	private:
		// In seconds
		static float s_ReactionDelay;
		int m_Steps = 0;

		NavData m_NavData;
	};
}
