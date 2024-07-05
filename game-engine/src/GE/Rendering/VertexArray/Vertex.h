#pragma once

#include <glm/glm.hpp>

namespace GE
{
	
	struct QuadVertex
	{
		// Position Globally
		glm::vec3 Position = glm::vec3(0.0f);

		glm::vec4 Color = glm::vec4(1.0f);
		glm::vec2 TextureCoord = glm::vec2(0.0f);
		int TextureIndex = 0;
		float TilingFactor = 1.0f;

		int EntityID = -1;
	};
	
	struct CircleVertex
	{
		// Position Globally
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec3 LocalPosition = glm::vec3(0.0f);

		glm::vec4 Color = glm::vec4(1.0f);
		float Radius = 0.5f; // Game Engine is 1x1, thus radius = 1/2;
		float Thickness = 1.0f; // 1: Full, 0: Empty
		float Fade = 0.0f; // Controls Blur

		int EntityID = -1;
	};
	
	struct LineVertex
	{
		// Position Globally
		glm::vec3 Position = glm::vec3(0.0f);

		glm::vec4 Color = glm::vec4(1.0f);

		int EntityID = -1;
	};
	
	struct TextVertex
	{
		// Position Globally
		glm::vec3 Position = glm::vec3(0.0f);

		glm::vec4 TextColor = glm::vec4(1.0f);
		glm::vec4 BGColor = glm::vec4(1.0f);
		glm::vec2 TextureCoord = glm::vec2(0.0f);

		int EntityID = -1;
	};
}