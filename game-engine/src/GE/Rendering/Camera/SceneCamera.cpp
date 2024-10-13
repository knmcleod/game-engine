#include "GE/GEpch.h"

#include "SceneCamera.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace GE
{
	SceneCamera::SceneCamera(float fov, float nearClip, float farClip, float aspectRatio)
		: Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip)), 
		p_FOV(fov), p_NearClip(nearClip), p_FarClip(farClip), p_AspectRatio(aspectRatio)
	{
		UpdateViewProjection();
	}

	SceneCamera::~SceneCamera()
	{

	}

	void SceneCamera::SetInfo(float fov, float nearClip, float farClip)
	{
		p_FOV = fov;
		p_NearClip = nearClip;
		p_FarClip = farClip;

		UpdateViewProjection();
	}

	const glm::quat SceneCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-p_Pitch, -p_Yaw, 0.0f));
	}

	void SceneCamera::CalculatePosition()
	{
		
	}

	void SceneCamera::OnUpdate(Timestep ts)
	{
		UpdateViewProjection();
	}

	void SceneCamera::UpdateViewProjection()
	{
		if (p_ViewportWidth == 0 || p_ViewportHeight == 0)
			return;

		p_AspectRatio = (float)p_ViewportWidth / (float)p_ViewportHeight;

		switch (p_ProjectionType)
		{
		case ProjectionType::Orthographic:
		{
			float orthoLeft = -p_FOV * p_AspectRatio * 0.5f;
			float orthoRight = p_FOV * p_AspectRatio * 0.5f;
			float orthoBottom = -p_FOV * 0.5f;
			float orthoTop = p_FOV * 0.5f;

			p_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, p_NearClip, p_FarClip);
		}
		break;
		case ProjectionType::Perspective:
		{
			p_Projection = glm::perspective(glm::radians(p_FOV), p_AspectRatio, p_NearClip, p_FarClip);
			
			CalculatePosition();
			glm::quat orientation = GetOrientation();
			p_ViewMatrix = glm::translate(Renderer::Get()->GetIdentityMat4(), p_Position) * glm::mat4(orientation);
		}
		break;
		}

		p_ViewMatrix = glm::inverse(p_ViewMatrix);
	}
}