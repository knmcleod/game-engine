#include "GE/GEpch.h"

#include "CameraController.h"

#include "GE/Core/Input/Input.h"
#include "GE/Scene/Components/Components.h"

namespace GE
{
	void CameraController::OnUpdate(Timestep timestep)
	{
		auto& translation = GetEntityComponent<TransformComponent>().Translation;
		float speed = 10.0f;
		//	Camera Movement
		if (Input::IsKeyPressed(GE_KEY_A))
		{
			translation.x -= speed * timestep;
		}
		else if (Input::IsKeyPressed(GE_KEY_D))
		{
			translation.x += speed * timestep;
		}

		if (Input::IsKeyPressed(GE_KEY_W))
		{
			translation.y += speed * timestep;
		}
		else if (Input::IsKeyPressed(GE_KEY_S))
		{
			translation.y -= speed * timestep;
		}

	}
}