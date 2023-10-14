#pragma once

#include "GE/Rendering/Texture/Texture.h"

namespace GE
{
	class Renderer2D
	{
	public:
		static void Init();
		static void ShutDown();

		static void Start(const OrthographicCamera& orthoCamera);
		static void End();

		static void FillQuadColor(const glm::vec3& position, const glm::vec2& size, const float rotation,
			const glm::vec4& color);
		static void FillQuadTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
			const Ref<Texture> texture, const glm::vec4& color = glm::vec4(1.0f));
	private:
		
	};
}