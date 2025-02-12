#pragma once

#include "GE/Core/Application/Window/Window.h"

#include <GLFW/glfw3.h>

namespace GE
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const Config& config);
		~WindowsWindow() override;

		inline float GetTime() const override { return (float)glfwGetTime(); }
		inline const uint64_t& GetWidth() const override { return m_Config.Width; }
		inline const uint64_t& GetHeight() const override { return m_Config.Height; }
		inline const std::pair<uint64_t, uint64_t>  GetPosition() const override { return { m_Config.PositionX, m_Config.PositionY }; }
		
		/*
		* Does not include window position or tabbar
		*/
		inline const std::pair<float, float> GetCursor() const override { return { m_Config.CursorX, m_Config.CursorY }; }
		
		bool IsVSync() const override { return m_Config.VSync; }
		bool IsFullscreen() const override;
		bool IsMouseActive() override;
		bool IsMousePressed(const Input::MouseCode& button) const override;
		bool IsKeyPressed(const Input::KeyCode& keycode) const override;

	private:
		inline void* GetNativeWindow() const override { return m_Window; }
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Config.EventCallback = callback; }

		void SetVSync(bool enabled) override;
		void SetFullscreen(bool fs) const override;
		Input::CursorMode GetCursorMode() override;
		void SetCursorMode(const Input::CursorMode& mode) override;
		void SetCursorShape(const CursorShape& shape) override;
		void SetCursorIcon(UUID textureHandle, uint32_t x, uint32_t y) override;
		void SetIcon(UUID textureHandle) override;

		void OnUpdate() override;

	private:
		Config m_Config;

		GLFWmonitor* m_Monitor = nullptr;
		GLFWwindow* m_Window = nullptr;
		GLFWcursor* m_Cursor = nullptr;

		Input::CursorMode m_CursorMode = Input::CursorMode::Normal;
	};
}

