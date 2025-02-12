#pragma once

#include "Event.h"
#include "GE/Core/Input/Input.h"

namespace GE
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y) : Event(false, Type::MouseMoved), m_MouseX(x), m_MouseY(y) {}
		
		const char* GetName() const override { return "MouseMoved"; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		inline const float& GetX() const { return m_MouseX; }
		inline const float& GetY() const { return m_MouseY; }

		EVENT_CLASS_TYPE(MouseMoved)
	private:
		float m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: Event(false, Type::MouseScrolled), m_XOffset(xOffset), m_YOffset(yOffset) {}

		const char* GetName() const override { return "MouseScrolled"; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
			return ss.str();
		}

		inline const float& GetXOffset() const { return m_XOffset; }
		inline const float& GetYOffset() const { return m_YOffset; }

		EVENT_CLASS_TYPE(MouseScrolled)
	private:
		float m_XOffset, m_YOffset;
	};

	class MousePressedEvent : public Event
	{
	public:
		MousePressedEvent(Input::MouseCode button) : Event(false, Type::MousePressed), m_Button(button){}

		inline const Input::MouseCode& GetButton() const { return m_Button; }

		const char* GetName() const override { return "MousePressed"; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MousePressedEvent: " << m_Button;
			return ss.str();
		}
		EVENT_CLASS_TYPE(MousePressed)
	private:
		Input::MouseCode m_Button;
	};

	class MouseReleasedEvent : public Event
	{
	public:
		MouseReleasedEvent(Input::MouseCode button)
			: Event(false, Type::MouseReleased), m_Button(button) {}
		
		inline const Input::MouseCode& GetButton() const { return m_Button; }

		const char* GetName() const override { return "MouseReleased"; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseReleased)
	private:
		Input::MouseCode m_Button;
	};
}