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

	void SceneCamera::CalculatePosition()
	{
		// Get Entity that parents CameraComponent, then get TRSC.Translation
	}

	void SceneCamera::SetViewport(uint32_t width, uint32_t height)
	{
		if (width > 0 && height > 0)
		{
			p_ViewportWidth = width; 
			p_ViewportHeight = height;

			p_AspectRatio = (float)p_ViewportWidth / (float)p_ViewportHeight;
		}
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
			cameraRotation = glm::rotate(identityMat4, p_Pitch, { 1, 0, 0 })
				* glm::rotate(identityMat4, p_Yaw, { 0, 1, 0 })
				* glm::rotate(identityMat4, p_Roll, { 0, 0, 1 });

			return;
		}
		break;
		}

		cameraTransform = glm::translate(identityMat4, p_Position) * cameraRotation;
		p_ViewMatrix = glm::inverse(cameraTransform);

	}
}