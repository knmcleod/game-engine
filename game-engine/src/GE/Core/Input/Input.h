#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace GE
{
	class Input
	{
	public:
		static bool IsKeyPressed(int keycode);
		static bool IsMouseButtonPressed(int button);
		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();

	};
}