#pragma once
#include <glm/glm.hpp>

namespace GE
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection) : m_Projection(projection) {}
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }
		
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

		const ProjectionType GetProjectionType() { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; }

	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;
	};
}