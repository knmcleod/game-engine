#include "GE/GEpch.h"

#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace GE
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetCamera(float size, float nearClip, float farClip)
	{
		m_CameraFOV = size;
		m_NearClip = nearClip;
		m_FarClip = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetViewport(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;

		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		switch (m_ProjectionType)
		{
		case ProjectionType::Orthographic:
		{
			float orthoLeft = -m_CameraFOV * m_AspectRatio * 0.5f;
			float orthoRight = m_CameraFOV * m_AspectRatio * 0.5f;
			float orthoBottom = -m_CameraFOV * 0.5f;
			float orthoTop = m_CameraFOV * 0.5f;

			m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_NearClip, m_FarClip);
			break;
		}
		case ProjectionType::Perspective:
		{
			m_Projection = glm::perspective(m_CameraFOV, m_AspectRatio, m_NearClip, m_FarClip);
			break;
		}
		}
	
	}
}