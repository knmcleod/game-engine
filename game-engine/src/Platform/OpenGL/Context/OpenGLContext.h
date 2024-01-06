#pragma once

#include "GE/Rendering/Context/Context.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

struct GLFWwindow;

namespace GE
{
	class OpenGLContext : public Context
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		
		// Initializes context using GLFW and GLAD
		virtual void Init() override;

		// Swaps Buffers using GLFW
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};

}