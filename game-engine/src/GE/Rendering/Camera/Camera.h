#pragma once

#include "GE/Core/Time/Timestep.h"
#include "GE/Core/Events/Event.h"

#include <glm/glm.hpp>

namespace GE
{
	class Camera
	{
		friend class Scene;
		friend class AssetSerializer;
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

		Camera() = default;
		Camera(const glm::mat4& projection) : p_Projection(projection) {}
		virtual ~Camera() = default;

		inline void SetProjectionType(const ProjectionType& type) { p_ProjectionType = type; }
		inline const ProjectionType& GetProjectionType() const { return p_ProjectionType; }
		
		inline const glm::mat4& GetProjection() const { return p_Projection; }

		/*
		* Returns view matrix(camera transform) * projection
		*/
		virtual const glm::mat4 GetViewProjection() const = 0;
		virtual const glm::vec2 GetViewport() = 0;
		virtual const float& GetNearClip() const = 0; 
		virtual const float& GetFarClip() const = 0;
		virtual const float& GetFOV() const = 0;
		
		/*
		* Sets view matrix then calls UpdateViewProjection()
		*/
		virtual void SetViewMatrix(const glm::mat4& transform) = 0;
		virtual void SetViewport(uint32_t width, uint32_t height) = 0;
		virtual void SetNearClip(float value) = 0;
		virtual void SetFarClip(float value) = 0;
		virtual void SetFOV(float value) = 0;
		virtual void SetInfo(float fov, float nearClip, float farClip) = 0;

		virtual void OnUpdate(Timestep ts) = 0;
		virtual void OnEvent(Event& e) = 0;

	protected:
		/*
		* Call after the following changes 
		*	Viewport or aspectRatio
		*	near or far clip
		*	FOV
		*/
		virtual void UpdateViewProjection() = 0;

	protected:
		ProjectionType p_ProjectionType = ProjectionType::Orthographic;
		glm::mat4 p_Projection = glm::mat4(1.0f);
	};
}