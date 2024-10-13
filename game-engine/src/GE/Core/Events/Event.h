#pragma once

#include <string>
#include <sstream>
#include <GE/Core/Core.h>

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
			MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
		};

		enum Category
		{
			None = 0,
			Application = BIT(0),
			Input = BIT(1),
			Keyboard = BIT(2),
			Mouse = BIT(3),
			MouseButton = BIT(4)
		};

		virtual Type GetEventType() const { return Type::None; }
		virtual const char* GetName() const { return nullptr; }
		virtual int GetCategoryFlags() const { return Category::None; }
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(Category category) const
		{
			return GetCategoryFlags() & category;
		}
		inline bool IsHandled() const { return m_Handled; }
		inline void SetHandled(bool status) { m_Handled = status; }
	private:
		bool m_Handled = false;

	};

#define EVENT_CLASS_TYPE(type) static Event::Type GetStaticType() {	return Event::Type::##type; }\
								virtual Event::Type GetEventType() const override {	return GetStaticType(); }\
								virtual const char* GetName() const override {	return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override {	return category; }

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
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;

	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}
