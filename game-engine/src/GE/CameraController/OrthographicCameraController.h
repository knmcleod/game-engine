#pragma once

#include "GE/Core/Timestep/Timestep.h"
#include "GE/Rendering/Camera/OrthographicCamera.h"

#include "GE/Input/Input.h"
#include "GE/Events/ApplicationEvent.h"
#include "GE/Events/MouseEvent.h"

namespace GE
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep timestep);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_OrthoCamera; }

		void SetZoomLevel(float level) { m_ZoomLevel = level; }
		float GetZoomLevel() { return m_ZoomLevel; }

	private:
		float m_AspectRatio = 1.0f;
		float m_ZoomLevel = 1.0f;

		bool m_Rotation;

		glm::vec3 m_CameraPosition = glm::vec3(0.0f);
		float m_CameraTranslationSpeed = 1.0f;

		float m_CameraRotation = 0.0f;
		float m_CameraRotationSpeed = 0.25;

		OrthographicCamera m_OrthoCamera;

		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	};
}