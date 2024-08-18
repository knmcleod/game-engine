#pragma once

#include <GE/Rendering/Camera/Camera.h>
#include <GE/Core/Events/MouseEvent.h>

namespace GE
{
	class EditorCamera : public Camera
	{
		friend class EditorLayer;
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float nearClip, float farClip, float aspectRatio = 1.0f);
		~EditorCamera() override = default;

		inline const glm::mat4& GetViewProjection() const override { return m_ViewProjection; }
		inline const glm::vec2 GetViewport() override { return glm::vec2(m_ViewportWidth, m_ViewportHeight); }

		inline const float& GetNearClip() const override { return m_NearClip; }
		inline const float& GetFarClip() const override { return m_FarClip; }
		inline const float& GetFOV() const override { return m_FOV; }
		
		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& e) override;

		inline const float& GetDistance() const { return m_Distance; }
		inline const float& GetPitch() const { return m_Pitch; }
		inline const float& GetYaw() const { return m_Yaw; }

		inline const glm::vec3& GetPosition() const { return m_Position; }
		
	private:
		void UpdateProjection() override;
		void UpdateView() override;

		inline void SetViewMatrix(const glm::mat4& transform) override { m_ViewMatrix = transform; UpdateView(); }
		inline void SetViewport(uint32_t width, uint32_t height) override;
		inline void SetNearClip(float value) override { m_NearClip = value;  UpdateProjection(); }
		inline void SetFarClip(float value) override { m_FarClip = value;  UpdateProjection(); }
		inline void SetFOV(float size) override { m_FOV = size;  UpdateProjection(); }
		
		glm::quat GetOrientation() const;
		glm::vec3 GetVertical() const;
		glm::vec3 GetHorizontal() const;
		glm::vec3 GetDepth() const;

		std::pair<float, float> GetPanSpeed() const;
		float GetRotationSpeed() const { return m_RotationSpeed; }
		float GetZoomSpeed() const;

		inline glm::vec3 CalculatePosition() const { return m_FocalPoint - GetDepth() * m_Distance; }

		void MousePan(const glm::vec2& delta);
		void MouseOrbit(const glm::vec2& delta);
		void MouseZoom(float delta);

		bool OnMouseScroll(MouseScrolledEvent& e);

	private:
		glm::mat4 m_ViewProjection = glm::mat4();
		glm::mat4 m_ViewMatrix = glm::mat4();
		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

		float m_FOV = 45.0f, m_AspectRatio = 1.0f;
		float m_NearClip = -1.0f, m_FarClip = 1.0f;

		float m_RotationSpeed = 0.5;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;
		float m_Distance = 10.0f;

		glm::vec3 m_Position = glm::vec3();
		glm::vec3 m_FocalPoint = glm::vec3();

		glm::vec2 m_InitialMousePosition = glm::vec2();
	};
}
