#pragma once
#include "GEpch.h"

#include "GE/Core.h"

#include "GE/Events/Event.h"
#include "GE/Events/ApplicationEvent.h"

#include "GE/Window.h"

#include "GE/Layers/LayerStack.h"

namespace GE
{

	class GE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& GetApplication() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;

		LayerStack m_LayerStack;

		static Application* s_Instance;
	};

	//Defined in Client
	Application* CreateApplication();
}

