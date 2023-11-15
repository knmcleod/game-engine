#pragma once
#include "Camera.h"
#include "GE/Core/Events/MouseEvent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace GE
{
	class EditorCamera : public Camera
	{
	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f;
		float m_NearClip = 0.1f, m_FarClip = 100.0f;
		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;
		float m_Distance = 10.0f;

		float m_RotationSpeed = 0.5;
		
		glm::mat4 m_IdentityMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };


		std::pair<float, float> GetPanSpeed() const;
		float GetRotationSpeed() const;
		float GetZoomSpeed() const;

		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);
		void MousePan(const glm::vec2& delta);
		void MouseOrbit(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;


	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewport(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		const glm::vec3& GetPosition() const { return m_Position; }
		
		glm::quat GetOrientation() const;

		glm::vec3 GetVertical() const;
		glm::vec3 GetHorizontal() const;
		glm::vec3 GetDepth() const;

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

	};
}
