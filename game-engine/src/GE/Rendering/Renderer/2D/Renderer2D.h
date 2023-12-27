#pragma once

#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Camera/Camera.h"
#include "GE/Rendering/Camera//Editor/EditorCamera.h"

#include "GE/Rendering/Textures/Texture.h"
#include "GE/Rendering/Textures/SubTexture.h"

#include "GE/Scene/Components/Components.h"

namespace GE
{
	class Renderer2D
	{
	public:
		static void Init();
		static void ShutDown();

		static void Start(const EditorCamera& camera);
		static void Start(const OrthographicCamera& camera);
		static void Start(const Camera& camera, const glm::mat4& transform);
		static void End();

		static void Flush();
		static void FlushAndReset();

		static void FillQuad(const glm::mat4& transform, const glm::vec4& color, const int entityID = -1);
		static void FillQuad(const glm::mat4& transform, const Ref<Texture2D>& texture,
			const float& tilingFactor = 1.0f, const glm::vec4& color = glm::vec4(1.0f), const int entityID = -1);

		static void FillQuadColor(const glm::vec3& position, const glm::vec2& size, const float rotation,
			const glm::vec4& color, const int entityID = -1);
		static void FillQuadTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
			const Ref<Texture2D>& texture, const float& tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f),
			const int entityID = -1);
		static void FillQuadSubTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
			const Ref<SubTexture2D>& subTexture, const float& tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f),
			const int entityID = -1);

		static void SetQuadData(const glm::mat4& transform, const float& textureIndex, const glm::vec2 textureCoords[4],
			const float& tilingFactor, const glm::vec4& color, const int entityID = -1);
		static void ResetQuadData();

		static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& component, int entityID);

#pragma region Statistics
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static Statistics GetStats();
		static void ResetStats();
#pragma endregion

	};
}