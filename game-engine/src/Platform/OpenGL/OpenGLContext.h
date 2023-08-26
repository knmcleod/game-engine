#pragma once

#include "GE/Renderer/Context.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

struct GLFWwindow;

namespace GE
{
	class OpenGLContext : public Context
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};

}