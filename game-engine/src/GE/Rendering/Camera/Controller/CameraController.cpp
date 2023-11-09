#include "GE/GEpch.h"
#include "CameraController.h"
#include "GE/Scene/Components/Components.h"
#include "GE/Core/Input/Input.h"
namespace GE
{
	void CameraController::OnUpdate(Timestep timestep)
	{
		auto& transform = GetEntityComponent<TransformComponent>().Transform;
		float speed = 10.0f;
		//	Camera Movement
		if (Input::IsKeyPressed(GE_KEY_A))
		{
			transform[3][0] -= speed * timestep;
		}
		else if (Input::IsKeyPressed(GE_KEY_D))
		{
			transform[3][0] += speed * timestep;
		}

		if (Input::IsKeyPressed(GE_KEY_W))
		{
			transform[3][1] += speed * timestep;
		}
		else if (Input::IsKeyPressed(GE_KEY_S))
		{
			transform[3][1] -= speed * timestep;
		}

	}
}