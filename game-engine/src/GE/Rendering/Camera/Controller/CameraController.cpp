#include "GE/GEpch.h"

#include "CameraController.h"

#include "GE/Asset/Assets/Scene/Components/Components.h"

#include "GE/Core/Application/Application.h"
#include "GE/Core/Input/Input.h"

#include "GE/Project/Project.h"

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
		if (Ref<Scene> runtimeScene = Project::GetRuntimeScene())
		{
			auto& translation = runtimeScene->GetComponent<TransformComponent>(*this).Translation;
			float speed = 10.0f;
			//	Camera Movement
			if (Application::IsKeyPressed(Input::KEY_A))
			{
				translation.x -= speed * timestep;
			}
			else if (Application::IsKeyPressed(Input::KEY_D))
			{
				translation.x += speed * timestep;
			}

			if (Application::IsKeyPressed(Input::KEY_W))
			{
				translation.y += speed * timestep;
			}
			else if (Application::IsKeyPressed(Input::KEY_S))
			{
				translation.y -= speed * timestep;
			}
		}
	}
}