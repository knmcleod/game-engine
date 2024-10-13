#pragma once

#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	class OpenGLRenderer : public Renderer
	{
	public:
		OpenGLRenderer();
		~OpenGLRenderer() override;

		const API& GetAPI() override { return s_Data.RenderAPI; }
		const Statistics& GetStats() override;
		void ClearStats() override;

		void CreateData() override;
		void ClearData() override;
		// Clears Color, Depth & Stencil GLBuffers
		void ClearBuffers() override;

		void Start(const Camera& camera) override;
		void End() override;
		void Flush() override;

		Ref<Texture2D> GetWhiteTexture() override { return s_Data.spriteData.EmptyTexture; }
		const glm::mat4& GetIdentityMat4() override { return s_Data.IdentityMat4; }

		void SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height) override;
		void SetClearColor(const glm::vec4& color) override;
		void SetLineWidth(float thickness) override;
		const float& GetLineWidth() override;

		void DrawLines(Ref<VertexArray> vertexArray, uint32_t vertexCount) override;
		void DrawIndices(Ref<VertexArray> vertexArray, uint32_t indexCount) override;

		// Sprite/Quad
		void DrawQuadColor(const glm::mat4& transform, const glm::vec4& color, const uint32_t& entityID) override;
		void DrawQuadTexture(const glm::mat4& transform, Ref<Texture2D> texture, const float& tilingFactor = 1.0f,
			const glm::vec4& color = glm::vec4(1.0f), const uint32_t& entityID = -1) override;
		void DrawQuadSubTexture(const glm::mat4& transform, Ref<SubTexture2D> subTexture, const float& tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f), const uint32_t& entityID = -1);
		void DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& component, const uint32_t& entityID) override;

		// Circle
		void DrawCircle(const glm::mat4& transform, const glm::vec4& color, const float& radius,
			const float& thickness, const float& fade, const uint32_t& entityID) override;
		void DrawCircle(const glm::mat4& transform, const CircleRendererComponent& component, const uint32_t& entityID) override;

		// Line/Rectangle
		void DrawRectangle(const glm::mat4& transform, const glm::vec4& color, const uint32_t& entityID) override;
		void DrawLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color,
			const uint32_t& entityID) override;

		// Text
		void DrawString(const glm::mat4& transform, const std::string& text, Ref<Font> font,
			const glm::vec4& texColor, const glm::vec4& bgColor, const float& kerningOffset,
			const float& lineHeightOffset, const uint32_t& entityID) override;
		void DrawString(const glm::mat4& transform, const TextRendererComponent& component, const uint32_t& entityID) override;

	private:
		// Sprite/Quad
		void SetQuadData(const glm::mat4& transform, const uint32_t& textureIndex, const glm::vec2 textureCoords[4],
			const float& tilingFactor, const glm::vec4& color,const uint32_t& entityID) override;
		void ResetQuadData() override;

		// Circle
		void SetCircleData(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), const float& radius = 0.5f,
			const float& thickness = 1.0f, const float& fade = 0.0f, const uint32_t& entityID = -1) override;
		void ResetCircleData() override;

		// Line/Rectangle
		void SetLineData(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const uint32_t& entityID) override;
		void ResetLineData() override;

		// Text
		void SetTextData(const glm::mat4& transform, const glm::vec2& minTextureCoord, const glm::vec2& maxTextureCoord,
			const glm::vec2& minQuadPlane, const glm::vec2& maxQuadPlane, Ref<Font> font,
			const glm::vec4& texColor, const glm::vec4& bgColor, const uint32_t& entityID) override;
		void ResetTextData() override;

	private:
		static Data s_Data;
	};
}

