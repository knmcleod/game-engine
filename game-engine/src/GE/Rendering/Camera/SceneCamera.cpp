#include "GE/GEpch.h"

#include "SceneCamera.h"

#include "GE/Rendering/Renderer/2D/Renderer2D.h"

namespace GE
{
	SceneCamera::SceneCamera(float fov, float nearClip, float farClip, float aspectRatio) : m_FOV(fov), m_NearClip(nearClip), m_FarClip(farClip), m_AspectRatio(aspectRatio)
	{
		UpdateView();
	}

	void SceneCamera::OnUpdate(Timestep ts)
	{
		UpdateView();
	}

	void SceneCamera::OnEvent(Event& e)
	{
	}

	void SceneCamera::UpdateProjection()
	{
		m_AspectRatio = (float)m_ViewportWidth / (float)m_ViewportHeight;

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
		UpdateProjection();

		m_ViewProjection = p_Projection * glm::inverse(m_ViewTransform);
	}
}