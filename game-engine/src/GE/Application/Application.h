#pragma once

#include "GE/Core.h"

#include "GE/Window/Window.h"
#include "GE/Window/Layers/LayerStack.h"
#include "GE/Window/Layers/imgui/ImGuiLayer.h"
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

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

	private:
		static Application* s_Instance;

		bool m_Running = true;

		std::unique_ptr<Window> m_Window;
		LayerStack m_LayerStack;

		ImGuiLayer* m_ImGuiLayer;

		bool OnWindowClose(WindowCloseEvent& e);
	};

	//Defined in Client
	Application* CreateApplication();
}

