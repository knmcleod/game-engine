#pragma once

#include "Camera.h"

#include "GE/Core/Events/MouseEvent.h"
#include "GE/Core/Time/Time.h"

namespace GE
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float nearClip, float farClip);
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);
		~EditorCamera() override = default;

		inline const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		inline const glm::mat4& GetViewProjection() const override { return p_Projection * m_ViewMatrix; }
		inline const glm::vec2 GetViewport() override { return glm::vec2(m_ViewportWidth, m_ViewportHeight); }

		inline const float& GetNearClip() const override { return m_NearClip; }
		inline const float& GetFarClip() const override { return m_FarClip; }
		inline const float& GetFOV() const override { return m_FOV; }

		inline const float& GetDistance() const { return m_Distance; }
		inline const float& GetPitch() const { return m_Pitch; }
		inline const float& GetYaw() const { return m_Yaw; }

		inline const glm::vec3& GetPosition() const { return m_Position; }

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& e) override;
		
	private:
		inline void SetViewport(float width, float height) override { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }
		inline void SetNearClip(float value) override { m_NearClip = value; UpdateProjection(); }
		inline void SetFarClip(float value) override { m_FarClip = value; UpdateProjection(); }
		inline void SetFOV(float size) override { m_FOV = size; UpdateProjection(); }

		glm::quat GetOrientation() const override;
		glm::vec3 GetVertical() const override;
		glm::vec3 GetHorizontal() const override;
		glm::vec3 GetDepth() const override;

		glm::vec3 CalculatePosition() const override;
		void UpdateProjection() override;
		void UpdateView() override;

		bool OnMouseScroll(MouseScrolledEvent& e);

		std::pair<float, float> GetPanSpeed() const;
		float GetRotationSpeed() const;
		float GetZoomSpeed() const;

		void MousePan(const glm::vec2& delta);
		void MouseOrbit(const glm::vec2& delta);
		void MouseZoom(float delta);

	private:
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		float m_FOV = 45.0f, m_AspectRatio = 1.0f;
		float m_NearClip = -1.0f, m_FarClip = 1.0f;

		float m_RotationSpeed = 0.5;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;
		float m_Distance = 10.0f;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
	};
}
