#include "GE/GEpch.h"

#include "WindowsWindow.h"

#include "GE/Core/Events/ApplicationEvent.h"
#include "GE/Core/Events/KeyEvent.h"
#include "GE/Core/Events/MouseEvent.h"

#include "Platform/OpenGL/Context/OpenGLContext.h"

namespace GE
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		GE_CORE_ERROR("GLFW Error {0}: {1}", error, description);
	}

	static bool s_GLFWInitialized = false;
	static int s_GLFWWindowCount = 0;

	WindowsWindow::WindowsWindow(const Config& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const Config& config)
	{
		GE_PROFILE_FUNCTION();

		m_Config = Config(config);
		GE_CORE_TRACE("Creating window {0}\n\tWidth, Height : ({1}, {2})", m_Config.Name, m_Config.Width, m_Config.Height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			GE_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)m_Config.Width, (int)m_Config.Height, m_Config.Name.c_str(), nullptr, nullptr);
		s_GLFWWindowCount++;

		glfwGetWindowSize(m_Window, (int*)&m_Config.Width, (int*)&m_Config.Height);
		glfwGetWindowPos(m_Window, (int*)&m_Config.PositionX, (int*)&m_Config.PositionY);

		m_Monitor = glfwGetPrimaryMonitor();

		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Config);

		SetVSync(true);

		//	Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window,
			[](GLFWwindow* window, int width, int height)
			{
				Config& data =  *(Config*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent e(width, height);
				data.EventCallback(e);
			}
		);

		glfwSetWindowCloseCallback(m_Window, 
			[](GLFWwindow* window)
			{
				Config& data = *(Config*)glfwGetWindowUserPointer(window);
				WindowCloseEvent e;
				data.EventCallback(e);
			}
		);

		glfwSetKeyCallback(m_Window,
			[](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				Config& data = *(Config*)glfwGetWindowUserPointer(window);
				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent e(key, 0);
						data.EventCallback(e);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent e(key);
						data.EventCallback(e);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent e(key, 1);
						data.EventCallback(e);
						break;
					}
				}
			}
		);
		
		//	Key Typed Event Callback
		glfwSetCharCallback(m_Window,
			[](GLFWwindow* window, unsigned int keycode)
			{
				Config& data = *(Config*)glfwGetWindowUserPointer(window);
				KeyTypedEvent e(keycode);
				data.EventCallback(e);
			}
		);

		glfwSetMouseButtonCallback(m_Window,
			[](GLFWwindow* window, int button, int action, int mods)
			{
				Config& data = *(Config*)glfwGetWindowUserPointer(window);
				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent e(button);
						data.EventCallback(e);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent e(button);
						data.EventCallback(e);
						break;
					}
				}
			}
		);

		glfwSetScrollCallback(m_Window,
			[](GLFWwindow* window, double xOffset, double yOffset)
			{
				Config& data = *(Config*)glfwGetWindowUserPointer(window);
				MouseScrolledEvent e((float)xOffset, (float)yOffset);
				data.EventCallback(e);
			}
		);

		glfwSetCursorPosCallback(m_Window,
			[](GLFWwindow* window, double xPos, double yPos)
			{
				Config& data = *(Config*)glfwGetWindowUserPointer(window);
				MouseMovedEvent e((float)xPos, (float)yPos);
				data.EventCallback(e);
			}
		
		);
	}

	void WindowsWindow::Shutdown()
	{
		GE_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
			glfwTerminate();

		delete[] m_Context;
	}

	void WindowsWindow::OnUpdate()
	{
		GE_PROFILE_FUNCTION();

		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Config.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Config.VSync;
	}

	void WindowsWindow::SetFullscreen(bool fs) const
	{
		if (IsFullscreen() == fs)
			return;

		if (fs)
		{
			// backup window position and window size
			glfwGetWindowPos(m_Window, (int*)&m_Config.PositionX, (int*)&m_Config.PositionY);
			glfwGetWindowSize(m_Window, (int*)&m_Config.Width, (int*)&m_Config.Height);

			// get resolution of monitor
			const GLFWvidmode* mode = glfwGetVideoMode(m_Monitor);

			// switch to full screen
			glfwSetWindowMonitor(m_Window, m_Monitor, 0, 0, mode->width, mode->height, 0);
		}
		else
		{
			// restore last window size and position
			glfwSetWindowMonitor(m_Window, nullptr, (int)m_Config.PositionX, (int)m_Config.PositionY,
				(int)m_Config.Width, (int)m_Config.Height, 0);
		}
	}

	bool WindowsWindow::IsFullscreen() const
	{
		return glfwGetWindowMonitor(m_Window) != nullptr;
	}
}
