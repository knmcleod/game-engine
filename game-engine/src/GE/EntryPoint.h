#pragma once
#ifdef GE_PLATFORM_WINDOWS
extern GE::Application* GE::CreateApplication();

int main(int argc, char** argv)
{
	GE::Log::Init();
	GE_CORE_WARN("Core Warn");

	auto app = GE::CreateApplication();
	app->Run();
	delete app;
}
#endif // GE_PLATFORM_WINDOWS
