#pragma once

#include "GE/Core/Events/Event.h"

namespace GE
{
	//	Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		/*
		* 
		*/
		struct Config
		{
			std::string Name = std::string("New Window");

			unsigned int Width = 1280, Height = 720;
			int PositionX = 0, PositionY = 0;

			bool VSync = true;

			EventCallbackFn EventCallback = nullptr;

			Config() = default;
			Config(const std::string& name, unsigned int width, unsigned int height,
				bool vSync, EventCallbackFn callback) 
				: Name(name), Width(width), Height(height), VSync(vSync), EventCallback(callback)
			{

			}
		};

		static Scope<Window> Create(const Config& config);

		virtual ~Window() {}

		virtual void* GetNativeWindow() const = 0;

		//Window attributes
		virtual float GetTime() const = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetFullscreen(bool fs) const = 0;
		virtual bool IsFullscreen() const = 0;

		virtual void Init(const Config& config) = 0;
		virtual void Shutdown() = 0;
		virtual void OnUpdate() = 0;
	};
}