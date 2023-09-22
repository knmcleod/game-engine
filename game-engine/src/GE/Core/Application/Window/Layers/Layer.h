#pragma once

#include "GE/Core/Timestep/Timestep.h"

#include "GE/Core/Events/Event.h"

namespace GE
{
	class  Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep timestep) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& e) {}

		inline const std::string& GetName() const { return m_DebugName;  }
	protected:
		std::string m_DebugName;
	};
}