#pragma once

#include "GE/Core/Application/Window/Window.h"
#include "GE/Rendering/Context/Context.h"

#include <GLFW/glfw3.h>

namespace GE
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const Config& config);
		~WindowsWindow() override;

		void OnUpdate() override;

		inline float GetTime() const override { return (float)glfwGetTime(); }
		inline unsigned int GetWidth() const override { return m_Config.Width; }
		inline unsigned int GetHeight() const override { return m_Config.Height; }

		inline void* GetNativeWindow() const override { return m_Window;  }

		//Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Config.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		void SetFullscreen(bool fs) const override;
		bool IsFullscreen() const override;

		void Init(const Config& props) override;
		void Shutdown() override;
	private:
		Config m_Config;

		GLFWmonitor* m_Monitor = nullptr;
		GLFWwindow* m_Window = nullptr;

		Context* m_Context = nullptr;
		
	};
}

