#include "EditorCamera.h"

#include <GE/Core/Input/Input.h>
#include <GE/Project/Project.h>
#include <GE/Rendering/Renderer/Renderer.h>

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace GE
{
	EditorCamera::EditorCamera(float fov, float nearClip, float farClip, float aspectRatio) 
		: SceneCamera(fov, nearClip, farClip, aspectRatio)
	{
		p_ProjectionType = ProjectionType::Perspective;
	}

	std::pair<float, float> EditorCamera::GetPanSpeed() const
	{
		float x = std::min(p_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(p_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::GetZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);

		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max = 100.0f
		return speed;
	}

	glm::vec3 EditorCamera::GetVertical() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	
	glm::vec3 EditorCamera::GetHorizontal() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	
	glm::vec3 EditorCamera::GetDepth() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}
	
	void EditorCamera::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(Input::KEY_LEFT_ALT))
		{
			const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;
			if (Input::IsMouseButtonPressed(Input::MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(Input::MOUSE_BUTTON_LEFT))
				MouseOrbit(delta);
			else if (Input::IsMouseButtonPressed(Input::MOUSE_BUTTON_RIGHT))
				MouseZoom(delta.y);
		}

		SceneCamera::OnUpdate(ts);
	}
	
	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(GE_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = GetPanSpeed();
		m_FocalPoint += -GetHorizontal() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetVertical() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseOrbit(const glm::vec2& delta)
	{
		float yawSign = GetVertical().y < 0 ? -1.0f : 1.0f;
		p_Yaw += yawSign * delta.x * GetRotationSpeed();
		p_Pitch += delta.y * GetRotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * GetZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetDepth();
			m_Distance = 1.0f;
		}
	}

}