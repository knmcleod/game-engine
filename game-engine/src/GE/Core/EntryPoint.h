#pragma once

#include "GE/Core/Application/Application.h"

#ifdef GE_PLATFORM_WINDOWS
	extern GE::Application* GE::CreateApplication();

	int main(int argc, char** argv)
	{
		GE::Log::Init();

		GE_PROFILE_BEGIN_SESSION("Startup", "GE_PROFILE_STARTUP.json");
		auto app = GE::CreateApplication();
		GE_PROFILE_END_SESSION();

		GE_PROFILE_BEGIN_SESSION("Runtime", "GE_PROFILE_RUNTIME.json");
		app->Run();
		GE_PROFILE_END_SESSION();

		GE_PROFILE_BEGIN_SESSION("Shutdown", "GE_PROFILE_SHUTDOWN.json");
		delete app;
		GE_PROFILE_END_SESSION();
	}
#endif // GE_PLATFORM_WINDOWS
