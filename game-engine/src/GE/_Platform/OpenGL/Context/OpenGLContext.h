#pragma once
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "GE/Rendering/Context/Context.h"

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