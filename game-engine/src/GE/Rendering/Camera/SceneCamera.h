#pragma once

#include "Camera.h"

namespace GE
{
	class SceneCamera : public Camera
	{
		friend struct CameraComponent;
		friend class Scene;
	public:
		SceneCamera() = default;
		SceneCamera(float fov, float nearClip, float farClip, float aspectRatio);
		~SceneCamera() override;

		inline const glm::mat4 GetViewProjection() const override { return p_Projection * p_ViewMatrix; }
		inline const glm::vec3& GetPosition() const { return p_Position; }
		inline const glm::vec2 GetViewport() override { return glm::vec2(p_ViewportWidth, p_ViewportHeight); }

		inline const float& GetNearClip() const override { return p_NearClip; }
		inline const float& GetFarClip() const override { return p_FarClip; }
		inline const float& GetFOV() const override { return p_FOV; }
		inline const float& GetPitch() const { return p_Pitch; }
		inline const float& GetYaw() const { return p_Yaw; }

		inline void SetViewMatrix(const glm::mat4& transform) override
		{
			p_ViewMatrix = transform; UpdateViewProjection();
		}

		inline void SetNearClip(float value) override { p_NearClip = value; }
		inline void SetFarClip(float value) override { p_FarClip = value; }
		inline void SetFOV(float size) override { p_FOV = size; }

		void SetInfo(float fov, float nearClip, float farClip) override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override {}

		inline void SetPosition(const glm::vec3& position) { p_Position = position; }

		virtual const glm::quat GetOrientation() const;
		virtual void CalculatePosition();

	protected:
		inline void SetViewport(uint32_t width, uint32_t height) override 
		{ 
			if ((width > 0 && height > 0) && (width != p_ViewportWidth || height != p_ViewportHeight))
			{
				p_ViewportWidth = width; p_ViewportHeight = height;
				UpdateViewProjection();
			}
		}

		/*
		* Updates aspectRatio if viewport width or height has changed
		* ProjectionType determines if glm::ortho or glm::perspective is used.
		*/
		virtual void UpdateViewProjection() override;

	protected:
		// Camera/Entity Transform
		glm::mat4 p_ViewMatrix = glm::mat4(1.0f);
		// Camera/Entity Translation
		glm::vec3 p_Position = glm::vec3(0.0f);

		uint32_t p_ViewportWidth = 0, p_ViewportHeight = 0;

		float p_FOV = 45.0f, p_AspectRatio = 0.0f, p_NearClip = -1.0f, p_FarClip = 1.0f;
		float p_Pitch = 0.0f, p_Yaw = 0.0f;

	};
}