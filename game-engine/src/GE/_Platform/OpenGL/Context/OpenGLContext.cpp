#include "GE/GEpch.h"

#include "OpenGLContext.h"

namespace GE
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
	{
		GE_CORE_ASSERT(windowHandle, "Window Handle is null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		GE_CORE_ASSERT(status, "Failed to initialize Glad!");

		GE_CORE_INFO("--	OpenGL Info		--");
		GE_CORE_INFO("	Vender: {0}", (void*)(glGetString(GL_VENDOR)));
		GE_CORE_INFO("	Renderer: {0}", (void*)(glGetString(GL_RENDERER)));
		GE_CORE_INFO("	Version: {0}", (void*)(glGetString(GL_VERSION)));
	
		#ifdef GE_ENABLE_ASSERTS
			int versionMajor;
			int versionMinor;
			glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
			glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
			GE_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "GE requires at least OpenGL version 4.5!")
		#endif // GE_ENABLE_ASSERTS

	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}
