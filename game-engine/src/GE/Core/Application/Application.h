#pragma once

#include "GE/Core/Core.h"
#include "GE/Core/Time/Time.h"

#include "GE/Core/Application/Window/Window.h"
#include "GE/Core/Application/Window/Layers/LayerStack.h"
#include "GE/Core/Application/Window/Layers/imgui/ImGuiLayer.h"

#include "GE/Core/Events/ApplicationEvent.h"

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

		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);
	protected:
		bool m_Running = true;
		bool m_Minimized = false;

		float m_LastFrameTime = 0.0f;

		LayerStack m_LayerStack;

		ImGuiLayer* m_ImGuiLayer;

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		static Application* s_Instance;
		Scope<Window> m_Window;
	};

	//Defined in Client
	Application* CreateApplication();
}

