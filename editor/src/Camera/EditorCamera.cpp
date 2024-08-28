#include "EditorCamera.h"

#include <GE/Core/Input/Input.h>
#include <GE/Rendering/Renderer/2D/Renderer2D.h>

namespace GE
{
	EditorCamera::EditorCamera(float fov, float nearClip, float farClip, float aspectRatio) : Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip)),
		m_FOV(fov), m_NearClip(nearClip), m_FarClip(farClip), m_AspectRatio(aspectRatio)
	{
		UpdateView();
	}

	std::pair<float, float> EditorCamera::GetPanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
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

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
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
	
	void EditorCamera::SetInfo(float fov, float nearClip, float farClip)
	{
		m_FOV = fov;
		m_NearClip = nearClip;
		m_FarClip = farClip;

		UpdateProjection();
	}

	void EditorCamera::SetViewport(uint32_t width, uint32_t height)
	{ 
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width; 
		m_ViewportHeight = height; 
		UpdateView(); 
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = (float)m_ViewportWidth / (float)m_ViewportHeight;
		p_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		UpdateProjection();

		m_Position = CalculatePosition();
		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(Renderer2D::GetIdentityMat4(), m_Position) * glm::mat4(orientation);

		m_ViewProjection = p_Projection * glm::inverse(m_ViewMatrix);
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

		UpdateView();
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
		UpdateView();
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
		m_Yaw += yawSign * delta.x * GetRotationSpeed();
		m_Pitch += delta.y * GetRotationSpeed();
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