#pragma once

#include "GE/Core/Time/Timestep.h"

#include "GE/Core/Events/Event.h"
#include "GE/Core/Events/ApplicationEvent.h"

namespace GE
{
	class Layer
	{
	public:
		Layer(const std::string& name = "New Layer");
		virtual ~Layer();

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(Timestep timestep) = 0;
		virtual void OnEvent(Event& e) = 0;
		virtual void OnWindowResize(WindowResizeEvent& e) = 0;
		virtual void OnImGuiRender() = 0;

		inline const std::string& GetName() const { return m_DebugName;  }
	protected:
		std::string m_DebugName;
	};
}