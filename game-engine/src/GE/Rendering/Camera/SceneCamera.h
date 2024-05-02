#pragma once

#include "Camera.h"

namespace GE
{
	class SceneCamera : public Camera
	{
		friend class Scene;
		friend class AssetSerializer;
	public:
		SceneCamera();
		SceneCamera(float fov, float nearClip, float farClip);
		~SceneCamera() override = default;

		inline const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		inline const glm::mat4& GetViewProjection() const override { return p_Projection * m_ViewMatrix; }

		inline const glm::vec2 GetViewport() override { return glm::vec2(m_ViewportWidth, m_ViewportHeight); }
		inline void SetViewport(float width, float height) override { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		inline const float& GetNearClip() const override { return m_NearClip; }
		inline void SetNearClip(float value) override { m_NearClip = value; UpdateProjection(); }

		inline const float& GetFarClip() const override { return m_FarClip; }
		inline void SetFarClip(float value) override { m_FarClip = value; UpdateProjection(); }

		inline const float& GetFOV() const override { return m_FOV; }
		inline void SetFOV(float size) override { m_FOV = size; UpdateProjection(); }

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& e) override;
	private:
		glm::quat GetOrientation() const override;
		glm::vec3 GetVertical() const override;
		glm::vec3 GetHorizontal() const override;
		glm::vec3 GetDepth() const override;

		glm::vec3 CalculatePosition() const override;

		void UpdateProjection() override;
		void UpdateView() override;

	private:
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		float m_FOV = 45.0f, m_AspectRatio = 1.0f;
		float m_NearClip = -1.0f, m_FarClip = 1.0f;

		float m_Pitch = 0.0f, m_Yaw = 0.0f;
		float m_Distance = 10.0f;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

	};
}