#pragma once

#include "GE/Window.h"
#include "GE/Renderer/Context.h"

#include <GLFW/glfw3.h>

namespace GE
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline virtual unsigned int GetWidth() const override { return m_Data.Width; }
		inline virtual unsigned int GetHeight() const override { return m_Data.Height; }

		inline virtual void* GetNativeWindow() const override { return m_Window;  }

		//Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override
		{
			m_Data.EventCallback = callback;
		}
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

	protected:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;
		Context* m_Context;

		struct WindowData
		{
			std::string Title;
			unsigned int Width = 1280, Height = 720;
			bool VSync;

			EventCallbackFn EventCallback;

		};

		WindowData m_Data;
	};
}

