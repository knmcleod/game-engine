#include "GE/GEpch.h"

#include "OrthographicCameraController.h"

namespace GE
{
	OrthographicCameraController::OrthographicCameraController(uint32_t width, uint32_t height, bool useRotation) : m_Rotation(useRotation)
	{
		m_AspectRatio = (float)width / (float)height;

		float camWidth = 8.0f;
		float bottom = -camWidth;
		float top = camWidth;
		float left = bottom * m_AspectRatio;
		float right = top * m_AspectRatio;
		m_OrthoCamera = (OrthographicCamera&)OrthographicCamera(left, right, bottom, top);
		
		m_OrthoCameraBounds.Bottom = bottom;
		m_OrthoCameraBounds.Top = top;
		m_OrthoCameraBounds.Right = right;
		m_OrthoCameraBounds.Left = left;
	}

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool useRotation)
		: m_AspectRatio(aspectRatio), m_Rotation(useRotation),
		m_OrthoCamera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
	{
		m_OrthoCameraBounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset();
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		RecalculateView();
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		ResizeBounds((float)e.GetWidth(), (float)e.GetHeight());
		return false;
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

	void OrthographicCameraController::ResizeBounds(float width, float height)
	{
		m_AspectRatio = width / height;
		RecalculateView();
	}

	void OrthographicCameraController::RecalculateView()
	{
		m_OrthoCameraBounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_OrthoCamera.SetProjection(m_OrthoCameraBounds.Left, m_OrthoCameraBounds.Right, m_OrthoCameraBounds.Bottom, m_OrthoCameraBounds.Top);
	}
}
