#pragma once

#include "GE/Core/Core.h"
#include "GE/Core/Timestep/Timestep.h"

#include "GE/Application/Window/Window.h"
#include "GE/Application/Window/Layers/LayerStack.h"
#include "GE/Application/Window/Layers/imgui/ImGuiLayer.h"

#include "GE/Events/ApplicationEvent.h"

#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/VertexArray/VertexArray.h"

#include "GE/Rendering/Camera/OrthographicCamera.h"

namespace GE
{

	class Application
	{
	public:
		Application();
		virtual ~Application();

		inline static Application& GetApplication() { return *s_Instance; };
		inline Window& GetWindow() { return *m_Window; };

		// Runs Window - Updates Layers and Timestep
		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

	private:
		static Application* s_Instance;

		bool m_Running = true;
		bool m_Minimized = false;

		std::unique_ptr<Window> m_Window;
		LayerStack m_LayerStack;

		ImGuiLayer* m_ImGuiLayer;

		float m_LastFrameTime = 0.0f;

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	};

	//Defined in Client
	Application* CreateApplication();
}

