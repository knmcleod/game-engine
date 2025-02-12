#include "EditorCamera.h"

#include <GE/Core/Application/Application.h>
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
		return glm::rotate(glm::quat(glm::vec3(-p_Pitch, -p_Yaw, 0.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	
	glm::vec3 EditorCamera::GetHorizontal() const
	{
		return glm::rotate(glm::quat(glm::vec3(-p_Pitch, -p_Yaw, 0.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	
	glm::vec3 EditorCamera::GetDepth() const
	{
		return glm::rotate(glm::quat(glm::vec3(-p_Pitch, -p_Yaw, 0.0f)), glm::vec3(0.0f, 0.0f, -1.0f));
	}
	
	void EditorCamera::OnUpdate(Timestep ts)
	{
		if (Application::IsKeyPressed(Input::KEY_LEFT_ALT))
		{
			const glm::vec2 mouse = Application::GetWindowCursor();
			glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;
			if (Application::IsMousePressed(Input::MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (Application::IsMousePressed(Input::MOUSE_BUTTON_LEFT))
				MouseOrbit(delta);
			else if (Application::IsMousePressed(Input::MOUSE_BUTTON_RIGHT))
				MouseZoom(delta.y);
		}

		UpdateViewProjection();
	}
	
	void EditorCamera::UpdateViewProjection()
	{
		if (p_ViewportWidth == 0 || p_ViewportHeight == 0)
			return;

		p_AspectRatio = (float)p_ViewportWidth / (float)p_ViewportHeight;

		const glm::mat4& identityMat4 = Renderer::IdentityMat4();
		glm::mat4 cameraTransform = glm::mat4(0.0f);
		glm::mat4 cameraRotation = glm::mat4(1.0f);
		switch (p_ProjectionType)
		{
		case ProjectionType::Orthographic:
		{
			float orthoLeft = -p_FOV * p_AspectRatio * 0.5f;
			float orthoRight = p_FOV * p_AspectRatio * 0.5f;
			float orthoBottom = -p_FOV * 0.5f;
			float orthoTop = p_FOV * 0.5f;

			p_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, p_NearClip, p_FarClip);

			cameraRotation = glm::rotate(identityMat4, p_Roll, { 0, 0, 1 });
		}
		break;
		case ProjectionType::Perspective:
		{
			p_Projection = glm::perspective(glm::radians(p_FOV), p_AspectRatio, p_NearClip, p_FarClip);

			CalculatePosition();
			cameraRotation = glm::rotate(identityMat4, -p_Pitch, { 1, 0, 0 })
				* glm::rotate(identityMat4, -p_Yaw, { 0, 1, 0 })
				* glm::rotate(identityMat4, -p_Roll, { 0, 0, 1 });
		}
		break;
		}

		cameraTransform = glm::translate(identityMat4, p_Position) * cameraRotation;
		p_ViewMatrix = glm::inverse(cameraTransform);
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