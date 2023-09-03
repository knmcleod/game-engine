#pragma once

#include <GLFW/glfw3.h>

#include "GE/Application/Application.h"
#include "GE/Input/Input.h"

namespace GE
{
	class WindowsInput : public Input
	{
	protected:
		// Returns true if given keycode key is pressed
		virtual bool IsKeyPressedImpl(int keycode) override;

		// Returns true if given button code is pressed
		virtual bool IsMouseButtonPressedImpl(int button) override;

		// Returns mouse x position
		virtual float GetMouseXImpl() override;
		// Returns mouse y position
		virtual float GetMouseYImpl() override;
		// Returns mouse position
		virtual std::pair<float, float> GetMousePosImpl() override;
	};
}
