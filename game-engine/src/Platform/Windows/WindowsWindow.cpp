#include "GE/GEpch.h"

#include "WindowsWindow.h"

#include "GE/Core/Events/ApplicationEvent.h"
#include "GE/Core/Events/KeyEvent.h"
#include "GE/Core/Events/MouseEvent.h"

#include "GE/Project/Project.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace GE
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		GE_CORE_ERROR("GLFW Error {0}: {1}", error, description);
	}

	static bool s_GLFWInitialized = false;
	static int s_GLFWWindowCount = 0;

	WindowsWindow::WindowsWindow(const Config& config)
	{
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

		glfwGetWindowPos(m_Window, (int*)&m_Config.PositionX, (int*)&m_Config.PositionY);

		m_Monitor = glfwGetPrimaryMonitor();

		glfwSetWindowUserPointer(m_Window, &m_Config);

		//	Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window,
			[](GLFWwindow* window, int width, int height)
			{
				Config& data = *(Config*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent e(width, height);
				data.EventCallback(e);
			}
		);

		glfwSetWindowPosCallback(m_Window,
			[](GLFWwindow* window, int x, int y)
			{
				Config& data = *(Config*)glfwGetWindowUserPointer(window);
				data.PositionX = x;
				data.PositionY = y;

				// TODO : Window Moved Event(?)
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
					KeyPressedEvent e((Input::KeyCode)key, 0);
					data.EventCallback(e);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent e((Input::KeyCode)key);
					data.EventCallback(e);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent e((Input::KeyCode)key, 1);
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
				KeyTypedEvent e((Input::KeyCode)keycode);
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
					Input::MouseCode mouseCode = (Input::MouseCode)button;
					data.MouseButton = mouseCode;
					MousePressedEvent e(mouseCode);
					data.EventCallback(e);
					break;
				}
				case GLFW_RELEASE:
				{
					data.MouseButton = Input::MOUSE_BUTTON_NONE;
					MouseReleasedEvent e((Input::MouseCode)button);
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
				data.CursorX = (float)xPos;
				data.CursorY = (float)yPos;

				// Combined window & cursor positions for Moved Event
				float x = data.PositionX + (float)xPos;
				float y = data.PositionY + (float)yPos;

				MouseMovedEvent e(x, y);
				data.EventCallback(e);
			}

		);

		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		GE_CORE_ASSERT(status, "Failed to initialize Glad!");

		GE_CORE_TRACE("--	OpenGL Info		--");
		GE_CORE_TRACE("	Vender: {0}", (char*)(glGetString(GL_VENDOR)));
		GE_CORE_TRACE("	Renderer: {0}", (char*)(glGetString(GL_RENDERER)));
		GE_CORE_TRACE("	Version: {0}", (char*)(glGetString(GL_VERSION)));

		int versionMajor;
		int versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
		GE_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "GE requires at least OpenGL version 4.5!")

			glEnable(GL_DEPTH);
		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC0_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Renderer::Create();

		SetVSync(true);

	}

	WindowsWindow::~WindowsWindow()
	{
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
			glfwTerminate();
	}

	bool WindowsWindow::IsFullscreen() const
	{
		return glfwGetWindowMonitor(m_Window) != nullptr;
	}
	bool WindowsWindow::IsMouseActive()
	{
		// TODO :
		return false;
	}
	bool WindowsWindow::IsMousePressed(const Input::MouseCode& button) const
	{
		auto state = glfwGetMouseButton(m_Window, button);
		return state == GLFW_PRESS;
	}

	bool WindowsWindow::IsKeyPressed(const Input::KeyCode& keycode) const
	{
		auto state = glfwGetKey(m_Window, keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Config.VSync = enabled;
	}

	void WindowsWindow::SetFullscreen(bool fs) const
	{
		if (IsFullscreen() == fs)
			return;

		if (fs)
		{
			// get resolution of monitor
			const GLFWvidmode* mode = glfwGetVideoMode(m_Monitor);
			// switch to full screen
			glfwSetWindowMonitor(m_Window, m_Monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			// restore last window size and position
			glfwSetWindowMonitor(m_Window, nullptr, (int)m_Config.PositionX, (int)m_Config.PositionY + 34,
				(int)m_Config.Width, (int)m_Config.Height, GLFW_DONT_CARE);

		}
	}

	static Input::CursorMode GLFWInputModeToCursorMode(int mode)
	{
		switch (mode)
		{
		case GLFW_CURSOR_NORMAL:
			return Input::CursorMode::Normal;
		case GLFW_CURSOR_CAPTURED:
			return Input::CursorMode::Captured;
		case GLFW_CURSOR_HIDDEN:
			return Input::CursorMode::Hidden;
		case GLFW_CURSOR_DISABLED:
			return Input::CursorMode::Disabled;
		
		}
		return Input::CursorMode::None;
	}
	Input::CursorMode WindowsWindow::GetCursorMode()
	{
		return GLFWInputModeToCursorMode(glfwGetInputMode(m_Window, GLFW_CURSOR));
	}

	static int GetGLFWInputMode(const Input::CursorMode& mode)
	{
		switch (mode)
		{
		case Input::CursorMode::Normal:
			return GLFW_CURSOR_NORMAL;
			break;
		case Input::CursorMode::Captured:
			return GLFW_CURSOR_NORMAL;
			break;
		case Input::CursorMode::Hidden:
			return GLFW_CURSOR_HIDDEN;
			break;
		case Input::CursorMode::Disabled:
			return GLFW_CURSOR_DISABLED;
			break;
		default:
			GE_CORE_WARN("Couldn't get GLFWInputMode. Returning GLFW_CURSOR_NORMAL.");
			break;
		}
		return GLFW_CURSOR_NORMAL;
	}
	void WindowsWindow::SetCursorMode(const Input::CursorMode& mode)
	{
		if (m_CursorMode == mode)
			return;
		m_CursorMode = mode;
		glfwSetInputMode(m_Window, GLFW_CURSOR, GetGLFWInputMode(m_CursorMode));
	}

	static int GetGLFWCursorShape(const Window::CursorShape& mode)
	{
		// TODO : Update compatiablity 
		switch (mode)
		{
		case Window::CursorShape::Arrow:
			return GLFW_ARROW_CURSOR;
			break;
		case Window::CursorShape::IBeam:
			return GLFW_IBEAM_CURSOR;
			break;
		case Window::CursorShape::Crosshair:
			return GLFW_CROSSHAIR_CURSOR;
			break;
		case Window::CursorShape::Hand:
			return GLFW_HAND_CURSOR;
			break;
		case Window::CursorShape::HorizontalResize:
			return GLFW_HRESIZE_CURSOR;
			break;
		case Window::CursorShape::VerticalResize:
			return GLFW_VRESIZE_CURSOR;
			break;
		case Window::CursorShape::None:
		default:
			GE_CORE_WARN("Couldn't get GLFWCursorShape. Returning GLFW_ARROW_CURSOR");
			break;
		}
		return GLFW_ARROW_CURSOR;
	}
	void WindowsWindow::SetCursorShape(const CursorShape& shape)
	{
		if (m_Cursor = glfwCreateStandardCursor(GetGLFWCursorShape(shape)))
			glfwSetCursor(m_Window, m_Cursor);
	}

	void WindowsWindow::SetCursorIcon(UUID textureHandle, uint32_t x, uint32_t y)
	{
		if (Ref<Texture2D> cursorIcon = Project::GetAsset<Texture2D>(textureHandle))
		{
			GLFWimage image = GLFWimage();
			image.width = cursorIcon->GetWidth();
			image.height = cursorIcon->GetHeight();
			image.pixels = cursorIcon->GetConfig().TextureBuffer.As<unsigned char>();
			m_Cursor = glfwCreateCursor(&image, x, y);
			glfwSetCursor(m_Window, m_Cursor);
		}
	}

	void WindowsWindow::SetIcon(UUID textureHandle)
	{
		if (Ref<Texture2D> textureAsset = Project::GetAsset<Texture2D>(textureHandle))
		{
			if (textureAsset->GetConfig().TextureBuffer)
			{
				GLFWimage icon = GLFWimage();
				icon.width = textureAsset->GetWidth();
				icon.height = textureAsset->GetHeight();
				icon.pixels = textureAsset->GetConfig().TextureBuffer.As<unsigned char>();
				glfwSetWindowIcon(m_Window, 1, &icon); // Icon will be flipped because Texture2D is bottom-top, but GLFWimage is top-bottom
			}
		}
	}

	void WindowsWindow::OnUpdate()
	{
		GE_PROFILE_FUNCTION();

		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}
}
