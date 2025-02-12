#pragma once

#include "Event.h"
#include "GE/Core/Input/Input.h"

namespace GE
{
	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(Input::KeyCode key, int repeatCount) 
			: Event(false, Type::KeyPressed), m_KeyCode(key), m_RepeatCount(repeatCount) {}

		inline const Input::KeyCode& GetKeyCode() const { return m_KeyCode; }
		inline int GetRepeatCount() const { return m_RepeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		Input::KeyCode m_KeyCode;
		int m_RepeatCount;
	};

	class KeyReleasedEvent : public Event
	{
	public:
		KeyReleasedEvent(Input::KeyCode keycode)
			: Event(false, Type::KeyReleased), m_KeyCode(keycode){}

		inline const Input::KeyCode& GetKeyCode() { return m_KeyCode; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased)
	private:
		Input::KeyCode m_KeyCode;
	};

	class KeyTypedEvent : public Event
	{
	public:
		KeyTypedEvent(Input::KeyCode keycode) : Event(false, Type::KeyTyped), m_KeyCode(keycode) {}

		inline const Input::KeyCode& GetKeyCode() { return m_KeyCode; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyTyped)
	private:
		Input::KeyCode m_KeyCode;
	};
}
