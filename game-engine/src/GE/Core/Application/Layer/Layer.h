#pragma once

#include "GE/Core/Time/Timestep.h"

#include "GE/Core/Events/Event.h"
#include "GE/Core/Events/KeyEvent.h"
#include "GE/Core/Events/MouseEvent.h"

namespace GE
{
	// Forward declaration
	class Camera;
	class Entity;

	class Layer
	{
		friend class LayerStack;
	public:
		struct Config
		{
			// Set by LayerStack when inserted, matches index in stack. Zero is valid.
			uint64_t ID = 0;
			// Default true. Should be false if Layer is inherited.
			bool IsBase = true;

			Config() = default;
			Config(const Config& other) = default;
			Config(uint64_t id, bool isBase) : ID(id), IsBase(isBase)
			{

			}
		};

		Layer() = default;
		Layer(uint64_t id, bool isBase = true);
		virtual ~Layer();

		// Zero is valid
		inline const uint64_t& GetID() const { return p_Config.ID; }
		inline bool IsBase() const { return p_Config.IsBase; }

		virtual void OnAttach();
		virtual void OnDetach();
		virtual void OnUpdate(Timestep ts);

		virtual void OnEvent(Event& e);

	protected:
		void ClearID() { p_Config.ID = 0; }
		void SetID(uint64_t index) { p_Config.ID = index; }
		bool Validate(Entity entity) const;
		/*
		* Renders all Entities in RuntimeScene that are layer valid in using Layer::Validate(Entity)
		*/
		virtual void OnRender(Camera* camera);
		virtual bool OnKeyPressed(KeyPressedEvent& e);
		virtual bool OnMousePressed(MouseButtonPressedEvent& e);

	protected:
		Config p_Config = Config();
	};
}