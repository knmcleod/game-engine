#pragma once

namespace GE
{
	class  Input
	{
	public:
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		inline static std::pair<float, float> GetMousePos() { return s_Instance->GetMousePosImpl(); }

	protected:
		// Returns true if given keycode key is pressed
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		// Returns true if given button code is pressed
		virtual bool IsMouseButtonPressedImpl(int button) = 0;

		// Returns mouse x position
		virtual float GetMouseXImpl() = 0;
		// Returns mouse y position
		virtual float GetMouseYImpl() = 0;
		// Returns mouse position
		virtual std::pair<float, float> GetMousePosImpl() = 0;
	
	private:
		static Input* s_Instance;
	};
}