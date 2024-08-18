#pragma once

#include "GE/Asset/Assets/Font/Font.h"
#include "GE/Asset/Assets/Textures/Texture.h"
#include "GE/Asset/Assets/Textures/SubTexture.h"

#include "GE/Asset/Assets/Scene/Components/Components.h"

#include "GE/Rendering/VertexArray/Vertex.h"
#include "GE/Rendering/VertexArray/VertexArray.h"

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

			uint32_t GetTotalVertexCount() const { return SpawnCount * 4; }
			uint32_t GetTotalIndexCount() const { return SpawnCount * 6; }
		};

		static Statistics GetStats();
		static void ResetStats();
#pragma endregion

		struct QuadData
		{
			glm::vec4 Vertices[4];
			uint32_t IndexCount = 0;
			QuadVertex* VertexBufferBase = nullptr;
			QuadVertex* VertexBufferPtr = nullptr;

			Ref<VertexArray> VertexArray;
			Ref<VertexBuffer> VertexBuffer;
			Ref<Shader> Shader;
		};

		struct SpriteData
		{
			static const uint32_t MaxTextureSlots = 32;

			uint32_t TextureSlotIndex = 1;
			std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
			Ref<Texture2D> EmptyTexture;
		};

		struct CircleData
		{
			Ref<VertexArray> VertexArray;
			Ref<VertexBuffer> VertexBuffer;
			Ref<Shader> Shader;

			uint32_t IndexCount = 0;
			CircleVertex* VertexBufferBase;
			CircleVertex* VertexBufferPtr;
		};

		struct LineData
		{
			Ref<VertexArray> VertexArray;
			Ref<VertexBuffer> VertexBuffer;
			Ref<Shader> Shader;

			uint32_t IndexCount = 0;
			LineVertex* VertexBufferBase;
			LineVertex* VertexBufferPtr;
			float Width = 2.0f;
		};

		struct TextData
		{
			Ref<VertexArray> VertexArray;
			Ref<VertexBuffer> VertexBuffer;
			Ref<Shader> Shader;

			uint32_t IndexCount = 0;
			TextVertex* VertexBufferBase;
			TextVertex* VertexBufferPtr;

			Ref<Texture2D> AtlasTexture;
		};

		struct Data
		{
			static const uint32_t MaxSpawns = 10000;
			static const uint32_t MaxVertices = MaxSpawns * 4;
			static const uint32_t MaxIndices = MaxSpawns * 6;

			Statistics Stats;

			const glm::mat4 IdentityMat4 = glm::mat4(1.0f);

			QuadData quadData;
			SpriteData spriteData;
			CircleData circleData;
			LineData lineData;
			TextData textData;
		};

		static Ref<Texture2D> Renderer2D::GetWhiteTexture()
		{
			return s_Data.spriteData.EmptyTexture;
		}

		static const glm::mat4& Renderer2D::GetIdentityMat4()
		{
			return s_Data.IdentityMat4;
		}

		static void Init();
		static void ShutDown();

		static void Start(const Camera& camera);
		static void End();

		static void Flush();

		// Sprite/Quad
		static void DrawQuad(const glm::mat4& transform, 
			const glm::vec4& color, const int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, 
			Ref<Texture2D> texture, const float& tilingFactor = 1.0f,
			const glm::vec4& color = glm::vec4(1.0f), const int entityID = -1);
		static void DrawSprite(const glm::mat4& transform, 
			const SpriteRendererComponent& component, int entityID);
		
		// Circle
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, const float& radius, const float& thickness, const float& fade, const int entityID = -1);
		static void DrawCircle(const glm::mat4& transform, const CircleRendererComponent& component, const int entityID);

		// Line/Rectangle
		static const float& GetLineWidth() { return s_Data.lineData.Width; }
		static void DrawRectangle(const glm::mat4& transform, const glm::vec4& color, const int entityID = -1);
		static void DrawLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID = 0);
	
		// Text
		static void DrawString(const glm::mat4& transform, 
			const std::string& text, Ref<Font> font, 
			const glm::vec4& texColor, const glm::vec4& bgColor, 
			const float& kerningOffset, const float& lineHeightOffset,
			const int entityID = -1);
		static void DrawString(const glm::mat4& transform, 
			const TextRendererComponent& component, int entityID);

	private:
		// Sprite/Quad
		static void SetQuadData(const glm::mat4& transform, 
			const uint32_t& textureIndex, const glm::vec2 textureCoords[4],
			const float& tilingFactor, const glm::vec4& color, 
			const int entityID = -1);
		static void ResetQuadData();

		static void FillQuadColor(const glm::vec3& position, const glm::vec2& size, const float rotation,
			const glm::vec4& color, const int entityID = -1);
		static void FillQuadTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
			Ref<Texture2D> texture, const float& tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f),
			const int entityID = -1);
		static void FillQuadSubTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
			Ref<SubTexture2D> subTexture, const float& tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f),
			const int entityID = -1);
		
		// Circle
		static void SetCircleData(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), const float& radius = 0.5f,
			const float& thickness = 1.0f, const float& fade = 0.0f, const int entityID = -1);
		static void ResetCircleData();

		// Line/Rectangle
		static void SetLineData(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID);
		static void ResetLineData();
		static void SetLineWidth(float thickness) { s_Data.lineData.Width = thickness; }

		static void FillLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID = -1);
		static void FillRectangle(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const int entityID = -1);

		// Text
		static void SetTextData(const glm::mat4& transform, const glm::vec2& minTextureCoord, const glm::vec2& maxTextureCoord,
			const glm::vec2& minQuadPlane, const glm::vec2& maxQuadPlane, Ref<Font> font, const glm::vec4& texColor, const glm::vec4& bgColor, const int entityID);
		static void ResetTextData();
	private:
		static Data s_Data;
	};
}