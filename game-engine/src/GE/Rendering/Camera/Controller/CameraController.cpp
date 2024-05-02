#include "GE/GEpch.h"

#include "CameraController.h"

#include "GE/Asset/Assets/Scene/Components/Components.h"

#include "GE/Core/Input/Input.h"

namespace GE
{
	void CameraController::OnCreate()
	{
	}

	void CameraController::OnDestroy()
	{
	}

	void CameraController::OnUpdate(Timestep timestep)
	{
		auto& translation = GetComponent<TransformComponent>().Translation;
		float speed = 10.0f;
		//	Camera Movement
		if (Input::IsKeyPressed(KEY_A))
		{
			translation.x -= speed * timestep;
		}
		else if (Input::IsKeyPressed(KEY_D))
		{
			translation.x += speed * timestep;
		}

		if (Input::IsKeyPressed(KEY_W))
		{
			translation.y += speed * timestep;
		}
		else if (Input::IsKeyPressed(KEY_S))
		{
			translation.y -= speed * timestep;
		}

	}
}