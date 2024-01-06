#pragma once

#include "GE/Core/Application/Layers/Layer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

namespace GE
{
	class  ImGuiLayer : public Layer
	{
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;

	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;
		
		inline void BlockEvents(bool block) { m_BlockEvents = block; }

		void Begin();
		void End();

		void SetDarkTheme(ImGuiStyle& style);

	};
	
}

