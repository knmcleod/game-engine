#pragma once

#include "GE/Rendering/Camera/Camera.h"

namespace GE
{
	class SceneCamera : public Camera
	{
	private:
		float m_AspectRatio = 1.0f;
		float m_CameraFOV = 10.0f;
		float m_NearClip = -1.0f, m_FarClip = 1.0f;

		void RecalculateProjection();
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetViewport(uint32_t width, uint32_t height);

		float GetNearClip() const { return m_NearClip; }
		void SetNearClip(float value) { m_NearClip = value; RecalculateProjection(); }

		float GetFarClip() const { return m_FarClip; }
		void SetFarClip(float value) { m_FarClip = value; RecalculateProjection();	}

		float GetCameraFOV() const { return m_CameraFOV; }
		void SetCameraFOV(float size) { m_CameraFOV = size; RecalculateProjection(); }
		
		void SetCamera(float size, float nearClip, float farClip);
	};
}