#pragma once

#include "Camera.h"

namespace GE
{
	class SceneCamera : public Camera
	{
		friend struct CameraComponent;
		friend class Scene;
	public:
		SceneCamera() = default;
		SceneCamera(float fov, float nearClip, float farClip, float aspectRatio);
		~SceneCamera() override;

		inline const glm::mat4 GetViewProjection() const override { return p_Projection * p_ViewMatrix; }
		inline const glm::vec2 GetViewport() const override { return glm::vec2(p_ViewportWidth, p_ViewportHeight); }
		inline const float& GetAspectRatio() const override { return p_AspectRatio; }
		inline const float& GetNearClip() const override { return p_NearClip; }
		inline const float& GetFarClip() const override { return p_FarClip; }
		inline const float& GetFOV() const override { return p_FOV; }

		inline const glm::vec3& GetPosition() const override { return p_Position; }
		inline const glm::vec3 GetRotation() const override { return glm::vec3(p_Pitch, p_Yaw, p_Roll); }

		inline const float& GetPitch() const { return p_Pitch; }
		inline const float& GetYaw() const { return p_Yaw; }
		inline const float& GetRoll() const { return p_Roll; }

	protected:
		inline void SetPosition(const glm::vec3& position) { p_Position = position; }
		inline void SetRotation(const glm::vec3& rotation) { p_Pitch = rotation.x; p_Yaw = rotation.y; p_Roll = rotation.z; }
		inline void SetNearClip(float value) override { p_NearClip = value; }
		inline void SetFarClip(float value) override { p_FarClip = value; }
		inline void SetFOV(float size) override { p_FOV = size; }

		void SetViewport(uint32_t width, uint32_t height) override;

		/*
		* Updates aspectRatio if viewport width or height has changed
		* ProjectionType determines if glm::ortho or glm::perspective is used.
		* 
		*/
		virtual void UpdateViewProjection() override;
		virtual void CalculatePosition();

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override {}

	protected:
		// Camera/Entity Transform
		glm::mat4 p_ViewMatrix = glm::mat4(1.0f);
		// Ratio between w & h
		float p_AspectRatio = 0.0f;

		// Camera/Entity Translation
		glm::vec3 p_Position = glm::vec3(0.0f);
		// Scene w & h
		uint32_t p_ViewportWidth = 0, p_ViewportHeight = 0;

		//How many cells(1:1) can be seen in each direction
		float p_FOV = 45.0f;
		float p_NearClip = -1.0f, p_FarClip = 1.0f;

		// X Rotation Axis
		float p_Pitch = 0.0f;
		// Y Rotation Axis
		float p_Yaw = 0.0f;
		// Z Rotation Axis
		float p_Roll = 0.0f;
	};
}