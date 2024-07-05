#pragma once

#include "GE/Core/Application/Application.h"

#ifdef GE_PLATFORM_WINDOWS
	/*
	* Defined in Client Application
	* Example: EditorApplication.cpp
	*/ 
	extern GE::Application* GE::CreateApplication(Application::CommandLineArgs args);

	int main(int argc, char** argv)
	{
		GE::Log::Init();

		GE_PROFILE_BEGIN_SESSION("Startup", "GE_PROFILE_STARTUP.json");
		GE::Application* app = GE::CreateApplication({ argc, argv });
		GE_PROFILE_END_SESSION();

		GE_PROFILE_BEGIN_SESSION("Runtime", "GE_PROFILE_RUNTIME.json");
		app->Run();
		GE_PROFILE_END_SESSION();

		GE_PROFILE_BEGIN_SESSION("Shutdown", "GE_PROFILE_SHUTDOWN.json");
		delete app;
		GE_PROFILE_END_SESSION();

	}
#endif // GE_PLATFORM_WINDOWS
