#include "GE/GEpch.h"

#include "GE/_Platform/OpenGL/Context/OpenGLContext.h"

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
		GE_CORE_INFO("		Vender: {0}", (void*)glGetString(GL_VENDOR));
		GE_CORE_INFO("		Renderer: {0}", (void*)glGetString(GL_RENDERER));
		GE_CORE_INFO("		Version: {0}", (void*)glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}
