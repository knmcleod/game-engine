#pragma once

#include "GE/Core/Application/Layers/Layer.h"

namespace GE
{
	class ImGuiLayer : public Layer
	{
	private:
		bool m_BlockEvents = true;

	public:
		ImGuiLayer();
		virtual ~ImGuiLayer() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep timestep) override {}
		virtual void OnEvent(Event& e) override;
		virtual void OnWindowResize(WindowResizeEvent& e) override {}
		virtual void OnImGuiRender() override {}
		
		inline void BlockEvents(bool block) { m_BlockEvents = block; }

		void Begin();
		void End();

		uint32_t GetActiveWidgetID() const;

	};
	
}

