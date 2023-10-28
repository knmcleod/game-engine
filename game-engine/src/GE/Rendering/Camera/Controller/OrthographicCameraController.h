#pragma once

#include "GE/Core/Time/Time.h"
#include "GE/Rendering/Camera/OrthographicCamera.h"

#include "GE/Core/Input/Input.h"
#include "GE/Core/Events/ApplicationEvent.h"
#include "GE/Core/Events/MouseEvent.h"

namespace GE
{
	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool useRotation = true);
		OrthographicCameraController(uint32_t width, uint32_t height, bool useRotation = true);

		void OnUpdate(Timestep timestep);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_OrthoCamera; }

		float GetZoomLevel() { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; RecalculateView(); }

	private:
		float m_AspectRatio = 1.0f;
		float m_ZoomLevel = 1.0f;

		bool m_Rotation = false;

		glm::vec3 m_CameraPosition = glm::vec3(0.0f);
		float m_CameraTranslationSpeed = 1.0f;

		float m_CameraRotation = 0.0f;
		float m_CameraRotationSpeed = 0.25;

		OrthographicCamera m_OrthoCamera;
		OrthographicCameraBounds m_OrthoCameraBounds;

		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

		void RecalculateView();
	};
}