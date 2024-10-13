#pragma once

#include <GE/Rendering/Camera/SceneCamera.h>
#include <GE/Core/Events/MouseEvent.h>

namespace GE
{
	class EditorCamera : public SceneCamera
	{
		friend class EditorLayer;
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float nearClip, float farClip, float aspectRatio = 1.0f);
		~EditorCamera() override = default;
		
		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& e) override;

		inline const float& GetDistance() const { return m_Distance; }

	private:
		inline void CalculatePosition() override { p_Position = m_FocalPoint - GetDepth() * m_Distance; }

		glm::vec3 GetVertical() const;
		glm::vec3 GetHorizontal() const;
		glm::vec3 GetDepth() const;

		std::pair<float, float> GetPanSpeed() const;
		float GetRotationSpeed() const { return m_RotationSpeed; }
		float GetZoomSpeed() const;

		void MousePan(const glm::vec2& delta);
		void MouseOrbit(const glm::vec2& delta);
		void MouseZoom(float delta);

		bool OnMouseScroll(MouseScrolledEvent& e);

	private:
		float m_RotationSpeed = 0.5;
		float m_Distance = 10.0f;

		glm::vec3 m_FocalPoint = glm::vec3();

		glm::vec2 m_InitialMousePosition = glm::vec2();
	};
}
