#pragma once

#include "Camera.h"

namespace GE
{
	class SceneCamera : public Camera
	{
		friend class Scene;
		friend class AssetSerializer;
	public:
		SceneCamera() = default;
		SceneCamera(float fov, float nearClip, float farClip, float aspectRatio = 1.0f);
		~SceneCamera() override = default;

		inline const glm::mat4& GetViewProjection() const override { return m_ViewProjection; }
		inline const glm::vec2 GetViewport() override { return glm::vec2(m_ViewportWidth, m_ViewportHeight); }

		inline const float& GetNearClip() const override { return m_NearClip; }
		inline const float& GetFarClip() const override { return m_FarClip; }
		inline const float& GetFOV() const override { return m_FOV; }

		inline void SetViewMatrix(const glm::mat4& transform) override { m_ViewTransform = transform; UpdateView(); }
		inline void SetViewport(uint32_t width, uint32_t height) override { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }
		inline void SetNearClip(float value) override { m_NearClip = value; UpdateProjection(); }
		inline void SetFarClip(float value) override { m_FarClip = value;  UpdateProjection(); }
		inline void SetFOV(float size) override { m_FOV = size; UpdateProjection(); }

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& e) override;
	private:
		void UpdateProjection() override;
		void UpdateView() override;

	private:
		glm::mat4 m_ViewProjection = glm::mat4();
		glm::mat4 m_ViewTransform = glm::mat4();
		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

		float m_FOV = 45.0f, m_AspectRatio = 1.0f;
		float m_NearClip = -1.0f, m_FarClip = 1.0f;
	};
}