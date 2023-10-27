#pragma once

#include "GE/Rendering/RenderCommand.h"
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

		static void Flush();
		static void FlushAndReset();

		static void FillQuadColor(const glm::vec3& position, const glm::vec2& size, const float rotation,
			const glm::vec4& color);
		static void FillQuadTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
			const Ref<Texture2D> texture, const float& tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		
		static void SetQuadData(const glm::vec3& position, const glm::vec2& size, const float& rotation,
			const float& textureIndex, const float& tilingFactor, const glm::vec4& color);
		static void ResetQuadData();

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static Statistics GetStats();
		static void ResetStats();

	};
}