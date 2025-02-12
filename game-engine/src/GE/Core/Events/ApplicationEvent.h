#pragma once

#include "Event.h"

namespace GE
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height) : Event(false, Type::WindowResize), m_Width(width), m_Height(height) {}

		inline const uint32_t& GetWidth() const { return m_Width; }
		inline const uint32_t& GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
	private:
		uint32_t m_Width, m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() : Event(false, Type::WindowClose) {}

		EVENT_CLASS_TYPE(WindowClose)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() : Event(false, Type::AppTick) {}

		EVENT_CLASS_TYPE(AppTick)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() : Event(false, Type::AppUpdate) {}

		EVENT_CLASS_TYPE(AppUpdate)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() : Event(false, Type::AppRender) {}

		EVENT_CLASS_TYPE(AppRender)
	};

}
