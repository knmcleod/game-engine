#pragma once

#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Camera/Camera.h"
#include "GE/Rendering/Camera//Editor/EditorCamera.h"

#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/Textures/Texture.h"
#include "GE/Rendering/Textures/SubTexture.h"

#include "GE/Scene/Components/Components.h"

namespace GE
{
	class Renderer2D
	{
	public:
#pragma region Statistics
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t SpawnCount = 0;

			uint32_t GetTotalVertexCount() { return SpawnCount * 4; }
			uint32_t GetTotalIndexCount() { return SpawnCount * 6; }
		};

		static Statistics GetStats();
		static void ResetStats();
#pragma endregion

		struct QuadVertex
		{
			glm::vec3 GlobalPosition = glm::vec3(0.0f);
			glm::vec4 Color = glm::vec4(1.0f);
			glm::vec2 TextureCoord = glm::vec2(0.0f);
			float TextureIndex = 0;
			float TilingFactor = 1.0f;

			int EntityID = -1;
		};

		struct CircleVertex
		{
			glm::vec3 LocalPosition = glm::vec3(0.0f);
			glm::vec3 GlobalPosition = glm::vec3(0.0f);
			glm::vec4 Color = glm::vec4(1.0f);
			float Radius = 0.5f; // Game Engine is 1x1, thus radius = 1/2;
			float Thickness = 1.0f; // 1: Full, 0: Empty
			float Fade; // Controls Blur
			int EntityID = -1;
		};

		struct LineVertex
		{
			glm::vec3 Position = glm::vec3(0.0f);
			glm::vec4 Color = glm::vec4(1.0f);
			int EntityID = -1;
		};

		struct Renderer2DData
		{
			static const uint32_t MaxSpawns = 5000;
			static const uint32_t MaxVertices = MaxSpawns * 4;
			static const uint32_t MaxIndices = MaxSpawns * 6;

			// Sprite/Quad
			static const uint32_t MaxTextureSlots = 32;

			glm::vec4 QuadVertices[4];
			uint32_t QuadIndexCount = 0;
			QuadVertex* QuadVertexBufferBase = nullptr;
			QuadVertex* QuadVertexBufferPtr = nullptr;

			uint32_t TextureSlotIndex = 1;
			std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;

			Ref<VertexArray> QuadVertexArray;
			Ref<VertexBuffer> QuadVertexBuffer;
			Ref<Shader> QuadShader;
			Ref<Texture2D> EmptyTexture;

			// Circle
			Ref<VertexArray> CircleVertexArray;
			Ref<VertexBuffer> CircleVertexBuffer;
			Ref<Shader> CircleShader;

			uint32_t CircleIndexCount = 0;
			glm::vec4 CircleVertices[4];
			CircleVertex* CircleVertexBufferBase = nullptr;
			CircleVertex* CircleVertexBufferPtr = nullptr;

			// Line
			Ref<VertexArray> LineVertexArray;
			Ref<VertexBuffer> LineVertexBuffer;
			Ref<Shader> LineShader;

			uint32_t LineVertexCount = 0;
			LineVertex* LineVertexBufferBase = nullptr;
			LineVertex* LineVertexBufferPtr = nullptr;
			float LineWidth = 2.0f;
			//
			Statistics Stats;
		};

		static void Init();
		static void ShutDown();

		static void Start(const EditorCamera& camera);
		static void Start(const OrthographicCamera& camera);
		static void Start(const Camera& camera, const glm::mat4& transform);
		static void End();

		static void FlushLines();
		static void FlushCircles();
		static void FlushQuads();
		static void Flush();

		// Sprite/Quad

		static void SetQuadData(const glm::mat4& transform, const float& textureIndex, const glm::vec2 textureCoords[4],
			const float& tilingFactor, const glm::vec4& color, const int entityID = -1);
		static void ResetQuadData();

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
		static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& component, int entityID);
		
		// Circle

		static void SetCircleData(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), const float& radius = 0.5f,
			const float& thickness = 1.0f, const float& fade = 0.0f, const int entityID = -1);
		static void ResetCircleData();

		static void FillCircle(const glm::mat4& transform, const glm::vec4& color, const float radius, const float thickness, const float fade, const int entityID = -1);

		// Line/Rectangle
		
		static void SetLineData(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID);
		static void ResetLineData();
		static void SetLineWidth(float thickness);
		static float GetLineWidth();

		static void FillLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID = -1);
		static void FillRectangle(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const int entityID = -1);
		static void DrawRectangle(const glm::mat4& transform, const glm::vec4& color, const int entityID = -1);


		static const glm::mat4 s_IdentityMat4;

	};
}