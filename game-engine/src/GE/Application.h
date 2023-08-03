#pragma once
#include "GEpch.h"

#include "Core.h"

#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

#include "Window.h"

namespace GE
{

	class GE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	//Defined in Client
	Application* CreateApplication();
}

