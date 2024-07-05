#include "GE/GEpch.h"

#include "Window.h"

#ifdef GE_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif

namespace GE
{
	Scope<Window> Window::Create(const Config& config)
	{
#ifdef GE_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(config);
#else
		GE_CORE_ASSERT(false, "Unknown Platform!");
		return nullptr
#endif
	}
}