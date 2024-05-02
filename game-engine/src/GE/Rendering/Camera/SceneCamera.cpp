#include "GE/GEpch.h"

#include "SceneCamera.h"

namespace GE
{
	SceneCamera::SceneCamera()
	{
		UpdateProjection();
	}

	SceneCamera::SceneCamera(float fov, float nearClip, float farClip) : m_FOV(fov), m_NearClip(nearClip), m_FarClip(farClip)
	{
		UpdateProjection();
	}

	glm::quat SceneCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	glm::vec3 SceneCamera::GetVertical() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 SceneCamera::GetHorizontal() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 SceneCamera::GetDepth() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	void SceneCamera::OnUpdate(Timestep ts)
	{
		UpdateProjection();
	}

	void SceneCamera::OnEvent(Event& e)
	{
	}

	glm::vec3 SceneCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetDepth() * m_Distance;
	}

	void SceneCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;

		switch (p_ProjectionType)
		{
		case ProjectionType::Orthographic:
		{
			float orthoLeft = -m_FOV * m_AspectRatio * 0.5f;
			float orthoRight = m_FOV * m_AspectRatio * 0.5f;
			float orthoBottom = -m_FOV * 0.5f;
			float orthoTop = m_FOV * 0.5f;

			p_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_NearClip, m_FarClip);
			break;
		}
		case ProjectionType::Perspective:
		{
			p_Projection = glm::perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);
			break;
		}
		}
	}
	
	void SceneCamera::UpdateView()
	{
		m_Position = CalculatePosition();
		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(s_IdentityMatrix, m_Position) * glm::mat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}
}