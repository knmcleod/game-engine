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
		Ref<Texture2D> GetWhiteTexture() override { return s_Data.spriteData.EmptyTexture; }
		const glm::mat4& GetIdentityMat4() override { return s_Data.IdentityMat4; }
		const float& GetLineWidth() override;
		const QuadData::Vertices& GetVertices(const Pivot& pivot) override;
		glm::vec2 GetTextSize(UUID fontHandle, const glm::vec2& offset,
			const float& scalar, const float& kerningOffset, const float& lineHeightOffset, const std::string& text, const glm::vec2& padding) override;

		const Statistics& GetStats() override;
		void ClearStats() override;

		// Clears Color, Depth & Stencil GLBuffers
		void ClearBuffers() override;
	private:
		void CreateData() override;
		void ClearData() override;

		void Start(const Camera*& camera) override;
		void End() override;
		void Flush() override;

		void SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height) override;
		void SetClearColor(const glm::vec4& color) override;
		void SetLineWidth(float thickness) override;

		void DrawLines(Ref<VertexArray> vertexArray, uint32_t vertexCount) override;
		void DrawIndices(Ref<VertexArray> vertexArray, uint32_t indexCount) override;

		// Sprite/Quad
		void DrawQuadColor(const glm::mat4& transform, const Pivot& pivot, const glm::vec4& color, const uint32_t& entityID) override;
		void DrawQuadTexture(const glm::mat4& transform, const Pivot& pivot, Ref<Texture2D> texture, const float& tilingFactor,
			const glm::vec4& color, const uint32_t& entityID = -1) override;
		void DrawQuadSubTexture(const glm::mat4& transform, const Pivot& pivot, Ref<SubTexture2D> subTexture, const float& tilingFactor,
			const glm::vec4& tintColor, const uint32_t& entityID = -1) override;
		void DrawSprite(const glm::mat4& transform, const Pivot& pivot, const SpriteRendererComponent& src, const uint32_t& entityID) override;

		// Circle
		void DrawCircle(const glm::mat4& transform, const Pivot& pivot, const float& radius, const float& thickness, const float& fade,
			const glm::vec4& color, const uint32_t& entityID) override;
		void DrawCircle(const glm::mat4& transform, const Pivot& pivot, const CircleRendererComponent& crc, const uint32_t& entityID) override;

		// Line/Rectangle
		void DrawRectangle(const glm::mat4& transform, const Pivot& pivot, const glm::vec4& color, const uint32_t& entityID) override;
		void DrawLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color,
			const uint32_t& entityID) override;

		// Text
		void DrawString(const glm::mat4& transform, const std::string& text, Ref<Font> font,
			const glm::vec4& texColor, const glm::vec4& bgColor, const float& kerningOffset,
			const float& lineHeightOffset, const float& scalar, const glm::vec2& offset, const uint32_t& entityID) override;
		void DrawString(const glm::mat4& transform, const TextRendererComponent& trc, const uint32_t& entityID) override;

		// GUI
		void DrawButton(const glm::mat4& transform, const Pivot& pivot, const GUIButtonComponent& guiBC, const GUIState& state, const uint32_t& entityID) override;
		void DrawInputField(const glm::mat4& transform, const Pivot& pivot, const GUIInputFieldComponent& guiIFC, const GUIState& state, const uint32_t& entityID) override;
		void DrawSlider(const glm::mat4& transform, const Pivot& pivot, const GUISliderComponent& guiSC, const GUIState& state, const uint32_t& entityID) override;

		void DrawCheckbox(const glm::mat4& transform, const Pivot& pivot, const GUICheckboxComponent& guiCB, const GUIState& state, const uint32_t& entityID) override;

	private:
		// Sprite/Quad
		void SetQuadData(const glm::mat4& transform, const Pivot& pivot, const uint32_t& textureIndex, const glm::vec2 textureCoords[4],
			const float& tilingFactor, const glm::vec4& color, const uint32_t& entityID) override;
		void ResetQuadData() override;

		// Circle
		void SetCircleData(const glm::mat4& transform, const Pivot& pivot, const float& radius, const float& thickness, const float& fade,
			const glm::vec4& color, const uint32_t& entityID = -1) override;
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

