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

			uint64_t Width = 1280, Height = 720;
			uint64_t PositionX = 0, PositionY = 0;

			bool VSync = true;

			EventCallbackFn EventCallback = nullptr;

			Config() = default;
			Config(const std::string& name, uint64_t width, uint64_t height, bool vSync, EventCallbackFn callback) 
				: Name(name), Width(width), Height(height), VSync(vSync), EventCallback(callback)
			{

			}
		};

		static Scope<Window> Create(const Config& config);

		virtual ~Window() = default;

		virtual void* GetNativeWindow() const = 0;

		virtual float GetTime() const = 0;
		virtual const uint64_t& GetWidth() const = 0;
		virtual const uint64_t& GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void SetFullscreen(bool fs) const = 0;
		virtual bool IsFullscreen() const = 0;

		/*
		* Polls Events & Swaps Context Buffers
		*/
		virtual void OnUpdate() = 0;
	private:
		/*
		* Creates Window & Context. Initializes Event Callbacks.
		*/
		virtual void Init(const Config& config) = 0;
		/*
		* Destroys Window & Context
		*/
		virtual void Shutdown() = 0;
	};
}