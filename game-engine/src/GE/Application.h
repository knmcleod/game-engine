#pragma once
#include "Core.h"
#include "Events//Event.h"

namespace GE
{

	class GE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	//Defined in Client
	Application* CreateApplication();
}

