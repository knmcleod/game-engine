#pragma once

#include "GE/Core/Events/KeyEvent.h"
#include "GE/Core/Events/MouseEvent.h"

#include "GE/Rendering/Camera/Camera.h"

namespace GE
{
	// Forward declarations
	class Scene;
	class Entity;

	class Layer
	{
		friend class Application;
		friend class LayerStack;
	public:
		enum class Type
		{
			Debug = 0,
			Layer,
			GUI
		};
		struct Config
		{
			Config() = default;
			Config(const Config& other) = default;
			Config(uint64_t id) : ID(id)
			{

			}
			// Set by LayerStack when inserted, matches index in stack. Zero is valid.
			uint64_t ID = 0;
		};

		Layer() = default;
		Layer(uint64_t id);
		virtual ~Layer();

		// Zero is valid
		inline const uint64_t& GetID() const { return p_Config.ID; }
	protected:
		void ClearID() { p_Config.ID = 0; }
		void SetID(uint64_t index) { p_Config.ID = index; }

		/*
		* Renders entity & its children recursively. 
		* Intended to be used by OnRender(Ref<Scene>, const Camera*&).
		*
		* @param scene : runtime scene
		* @param entity : entity to render
		* @param translationOffset : offset from entity to parent
		* @param rotationOffset : offset from entity to parent
		*/
		virtual void RenderEntity(Ref<Scene> scene, Entity entity, glm::vec3& translationOffset, glm::vec3& rotationOffset);
		virtual void OnAttach(Ref<Scene> scene);
		virtual void OnDetach();

		/*
		* Should be used for Rendering.
		* Main Framebuffer will always be bound, unless unbound in method.
		*/
		virtual void OnUpdate(Ref<Scene> scene, Timestep ts);
		
		/*
		* Renders all Entities in RuntimeScene that are layer valid in using Layer::Validate(Entity)
		* Should be called in Layer::OnUpdate().
		*/
		virtual void OnRender(Ref<Scene> scene, const Camera*& camera);

		virtual void OnEvent(Event& e);
		virtual bool OnKeyPressed(KeyPressedEvent& e);
		virtual bool OnMousePressed(MousePressedEvent& e);
		virtual bool OnMouseMoved(MouseMovedEvent& e);
		virtual bool OnMouseScrolled(MouseScrolledEvent& e);
	protected:
		Config p_Config = Config();
	};

}