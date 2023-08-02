#include "Application.h"
#include "Log.h"
#include "Events/ApplicationEvent.h"

namespace GE
{
	Application::Application()
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		GE_TRACE(e);

		//run forever
		while (true);
	}
}