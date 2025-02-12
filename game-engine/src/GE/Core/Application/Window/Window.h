#pragma once

#include "GE/Core/Events/Event.h"
#include "GE/Core/Input/Input.h"
#include "GE/Core/UUID/UUID.h"

namespace GE
{
	//	Interface representing a desktop system based Window
	class Window
	{
		friend class Application;
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		/*
		* 
		*/
		struct Config
		{
			std::string Name = std::string("New Window");

			uint64_t Width = 1280, Height = 720; // Window Size
			uint64_t PositionX = 0, PositionY = 0; // Window Position
			float CursorX = 0.0f, CursorY = 0.0f;
			Input::MouseCode MouseButton = Input::MOUSE_BUTTON_NONE;
			Input::KeyCode KeyButton = Input::KEY_NONE;

			bool VSync = true;

			EventCallbackFn EventCallback = nullptr;

			Config() = default;
			Config(const std::string& name, uint64_t width, uint64_t height, bool vSync, EventCallbackFn callback) 
				: Name(name), Width(width), Height(height), VSync(vSync), EventCallback(callback)
			{

			}
		};

		enum class CursorShape
		{
			None = 0,
			Arrow,
			IBeam,
			Crosshair,
			Hand,
			HorizontalResize,
			VerticalResize
		};
		static Scope<Window> Create(const Config& config);

		virtual ~Window() = default;
	protected:
		virtual float GetTime() const = 0;
		virtual const uint64_t& GetWidth() const = 0;
		virtual const uint64_t& GetHeight() const = 0;
		virtual const std::pair<uint64_t, uint64_t> GetPosition() const = 0;
		virtual const std::pair<float, float>  GetCursor() const = 0;
		virtual bool IsVSync() const = 0;
		virtual bool IsFullscreen() const = 0;
		virtual bool IsMouseActive() = 0;
		virtual bool IsMousePressed(const Input::MouseCode& button) const = 0;
		virtual bool IsKeyPressed(const Input::KeyCode& button) const = 0;

		virtual void* GetNativeWindow() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void SetFullscreen(bool fs) const = 0;
		virtual Input::CursorMode GetCursorMode() = 0;
		virtual void SetCursorMode(const Input::CursorMode& mode) = 0;
		virtual void SetCursorShape(const CursorShape& shape) = 0;
		virtual void SetCursorIcon(UUID textureHandle, uint32_t x, uint32_t y) = 0;
		virtual void SetIcon(UUID textureHandle) = 0;

		/*
		* Polls Events & Swaps Context Buffers
		*/
		virtual void OnUpdate() = 0;
	};
}