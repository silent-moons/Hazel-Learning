#pragma once

#include <glm/glm.hpp>

namespace Hazel 
{
	class Camera
	{
	public:
		enum class ProjectionType { Orthographic = 0, Perspective = 1 };
	public:
		Camera() = default;
		Camera(const glm::mat4& projection) : m_Projection(projection) {}
		virtual ~Camera() = default;
		const glm::mat4& GetProjection() const { return m_Projection; }
		ProjectionType GetProjectionType() const { return m_ProjectionType; }
	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

	};
}