#pragma once

#include "GE/Core/Application/Window/Window.h"
#include "GE/Core/Application/Layers/LayerStack.h"

#include "GE/Core/Events/ApplicationEvent.h"

#include "GE/ImGui/ImGuiLayer.h"

namespace GE
{
	struct ApplicationSpecification
	{
		std::string Name = "Game Engine";
		std::string WorkingDirectory;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification specification);
		virtual ~Application();

		inline static Application& GetApplication() { return *s_Instance; };
		inline ApplicationSpecification& GetSpecification() { return m_Specification; };
		inline Window& GetWindow() { return *m_Window; };
		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		void OnEvent(Event& e);

		// Runs Window - Updates Layers and Timestep
		void Run();
		void Close();
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		static Application* s_Instance;
		ApplicationSpecification m_Specification;

		Scope<Window> m_Window;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		bool m_Running = true;
		bool m_Minimized = false;

		float m_LastFrameTime = 0.0f;

	};

	//Defined in Client
	Application* CreateApplication();
}

