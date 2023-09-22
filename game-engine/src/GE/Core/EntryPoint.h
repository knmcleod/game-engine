#pragma once

#include "GE/Core/Application/Application.h"

#ifdef GE_PLATFORM_WINDOWS
	extern GE::Application* GE::CreateApplication();

	int main(int argc, char** argv)
	{
		GE::Log::Init();

		auto app = GE::CreateApplication();
		app->Run();
		delete app;
	}
#endif // GE_PLATFORM_WINDOWS
