#pragma once

#include "GE/Core/Time/Time.h"

#include "GE/Core/Events/Event.h"
#include "GE/Core/Events/ApplicationEvent.h"

namespace GE
{
	class  Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = 0;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep timestep) {}
		virtual void OnEvent(Event& e) {}
		virtual void OnWindowResize(WindowResizeEvent& e) {}
		virtual void OnImGuiRender() {}

		inline const std::string& GetName() const { return m_DebugName;  }
	protected:
		std::string m_DebugName;
	};
}