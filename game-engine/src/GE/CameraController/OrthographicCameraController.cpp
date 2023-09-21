#include "GE/GEpch.h"

#include "OrthographicCameraController.h"

namespace GE
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_OrthoCamera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation)
	{

	}

	void OrthographicCameraController::OnUpdate(Timestep timestep)
	{
		//	Camera Movement
		if (Input::IsKeyPressed(GE_KEY_A))
		{
			m_CameraPosition.x -= m_CameraTranslationSpeed * timestep;
		}
		else if (Input::IsKeyPressed(GE_KEY_D))
		{
			m_CameraPosition.x += m_CameraTranslationSpeed * timestep;
		}

		if (Input::IsKeyPressed(GE_KEY_W))
		{
			m_CameraPosition.y += m_CameraTranslationSpeed * timestep;
		}
		else if (Input::IsKeyPressed(GE_KEY_S))
		{
			m_CameraPosition.y -= m_CameraTranslationSpeed * timestep;
		}

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(GE_KEY_Q))
			{
				m_CameraRotation -= m_CameraRotationSpeed * timestep;
			}
			else if (Input::IsKeyPressed(GE_KEY_E))
			{
				m_CameraRotation += m_CameraRotationSpeed * timestep;
			}

			m_OrthoCamera.SetRotation(m_CameraRotation);
		}

		m_OrthoCamera.SetPosition(m_CameraPosition);

		m_CameraTranslationSpeed = m_ZoomLevel;
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(GE_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(GE_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));

	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset();
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_OrthoCamera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio -= (float)e.GetWidth() / (float)e.GetHeight();
		m_OrthoCamera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}
}
