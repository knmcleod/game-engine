#pragma once

#include "GE/Core/Core.h"

#include <string>
#include <sstream>

namespace GE
{
	/*
	* Events are currently blocking;
	*	meaning when an event occurs, it's executed immediately.
	*/

	class Event
	{
		friend class EventDispatcher;
	public:
		enum class Type
		{
			None = 0,
			WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
			AppTick, AppUpdate, AppRender,
			KeyPressed, KeyReleased, KeyTyped,
			MousePressed, MouseReleased, MouseMoved, MouseScrolled
		};

		Event() = default;
		Event(bool isHandled, Type type) 
			: m_Handled(isHandled), m_Type(type) {} 
		
		inline bool IsHandled() const { return m_Handled; }
		inline void SetHandled(bool status) { m_Handled = status; }
		inline Type GetEventType() const { return m_Type; }

		virtual const char* GetName() const { return nullptr; }
		virtual std::string ToString() const { return GetName(); }
		
	protected:
		bool m_Handled = false;
		Type m_Type = Type::None;

	};
#define EVENT_CLASS_TYPE(type) static Event::Type GetStaticType() { return Event::Type::##type; }

	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event) : m_Event(event)
		{

		}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
			}
			return m_Event.m_Handled;
		}
	private:
		Event& m_Event;

	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}
