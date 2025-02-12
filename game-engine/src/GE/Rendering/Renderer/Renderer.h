#pragma once

#include "GE/Asset/Assets/Font/Font.h"
#include "GE/Asset/Assets/Scene/Entity.h"
#include "GE/Asset/Assets/Textures/Texture.h"
#include "GE/Asset/Assets/Textures/SubTexture.h"
#include "GE/Asset/Assets/Scene/Components/Components.h"

#include "GE/Core/Core.h"

#include "GE/Project/Project.h"

#include "GE/Rendering/Camera/Camera.h"
#include "GE/Rendering/VertexArray/Vertex.h"
#include "GE/Rendering/VertexArray/VertexArray.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace GE
{
	class Renderer
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
		
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t SpawnCount = 0;

			uint32_t GetTotalVertexCount() const { return SpawnCount * 4; }
			uint32_t GetTotalIndexCount() const { return SpawnCount * 6; }
		};

		struct QuadData
		{
			/*
			* 
			*/
			using Vertices = glm::vec4[4];

			std::map<Pivot, Vertices> PivotPoints = std::map<Pivot, Vertices>();

			uint32_t IndexCount = 0;
			QuadVertex* VertexBufferBase = nullptr;
			QuadVertex* VertexBufferPtr = nullptr;

			Ref<VertexArray> VertexArray = nullptr;
			Ref<VertexBuffer> VertexBuffer = nullptr;
			Ref<Shader> Shader = nullptr;
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
			CircleVertex* VertexBufferBase = nullptr;
			CircleVertex* VertexBufferPtr = nullptr;
		};

		struct LineData
		{
			Ref<VertexArray> VertexArray;
			Ref<VertexBuffer> VertexBuffer;
			Ref<Shader> Shader;

			uint32_t IndexCount = 0;
			LineVertex* VertexBufferBase = nullptr;
			LineVertex* VertexBufferPtr = nullptr;
			float Width = 2.0f;
		};

		struct TextData
		{
			Ref<VertexArray> VertexArray;
			Ref<VertexBuffer> VertexBuffer;
			Ref<Shader> Shader;

			uint32_t IndexCount = 0;
			TextVertex* VertexBufferBase = nullptr;
			TextVertex* VertexBufferPtr = nullptr;

			Ref<Texture2D> AtlasTexture;
		};

		struct Data
		{
			static const uint32_t MaxSpawns = 10000;
			static const uint32_t MaxVertices = MaxSpawns * 4;
			static const uint32_t MaxIndices = MaxSpawns * 6;

			// Default OpenGL
			static API RenderAPI;

			Statistics Stats;

			const glm::mat4 IdentityMat4 = glm::mat4(1.0f);

			QuadData quadData;
			SpriteData spriteData;
			CircleData circleData;
			LineData lineData;
			TextData textData;

		};

		/*
		* Creates instance of Renderer based on provided Renderer::API
		* 
		* @param api : render API. Default OpenGL.
		*/
		static void Create(API api = API::OpenGL);
		inline static const API& InstanceAPI() { return s_Instance->GetAPI(); }
		inline static void ClearColor(const glm::vec4& color) { s_Instance->SetClearColor(color); }
		inline static void CleanBuffers() { s_Instance->ClearBuffers(); }
		inline static void ResetStats() { s_Instance->ClearStats(); }
		inline static const Statistics& GetStatistics() { return s_Instance->GetStats(); }
		inline static void ResizeViewport(uint32_t xOff, uint32_t yOff, uint32_t w, uint32_t h) { s_Instance->SetViewport(xOff, yOff, w, h); }
		inline static const glm::mat4& IdentityMat4() { return s_Instance->GetIdentityMat4();}

		// Call before Draw 
		inline static void Open(const Camera*& camera) { s_Instance->Start(*&camera); }
		// Call after Draw
		inline static void Close() { s_Instance->End(); }

		inline static void DrawRect(const glm::mat4& transform, const Pivot& pivot, const glm::vec4& color, const uint32_t& entityID) { s_Instance->DrawRectangle(transform, pivot, color, entityID); }
		inline static void DrawSphere(const glm::mat4& transform, const Pivot& pivot, const float& radius, const float& thickness, const float& fade, const glm::vec4& color, const uint32_t& entityID) { s_Instance->DrawCircle(transform, pivot, radius, thickness, fade, color, entityID); }

		inline static void Draw(const glm::mat4& transform, const Pivot& pivot, const SpriteRendererComponent& src, const uint32_t& entityID) { s_Instance->DrawSprite(transform, pivot, src, entityID); }
		inline static void Draw(const glm::mat4& transform, const Pivot& pivot, const CircleRendererComponent& crc, const uint32_t& entityID) { s_Instance->DrawCircle(transform, pivot, crc, entityID); }
		inline static void Draw(const glm::mat4& transform, const TextRendererComponent& trc, const uint32_t& entityID) { s_Instance->DrawString(transform, trc, entityID); }

		inline static void Draw(const glm::mat4& transform, const Pivot& pivot, const GUIImageComponent& guiIC, const uint32_t& entityID) { s_Instance->DrawQuadTexture(transform, pivot, Project::GetAsset<Texture2D>(guiIC.TextureHandle), guiIC.TilingFactor, guiIC.Color, entityID); }
		inline static void Draw(const glm::mat4& transform, const Pivot& pivot, const GUIButtonComponent& guiBC, const GUIState& state, const uint32_t& entityID) { s_Instance->DrawButton(transform, pivot, guiBC, state, entityID); }
		inline static void Draw(const glm::mat4& transform, const Pivot& pivot, const GUIInputFieldComponent& guiIFC, const GUIState& state, const uint32_t& entityID) { s_Instance->DrawInputField(transform, pivot, guiIFC, state, entityID); }
		inline static void Draw(const glm::mat4& transform, const Pivot& pivot, const GUISliderComponent& guiSC, const GUIState& state, const uint32_t& entityID) { s_Instance->DrawSlider(transform, pivot, guiSC, state, entityID); }
		inline static void Draw(const glm::mat4& transform, const Pivot& pivot, const GUICheckboxComponent& guiCB, const GUIState& state, const uint32_t& entityID) { s_Instance->DrawCheckbox(transform, pivot, guiCB, state, entityID); }

		// TODO : Image, ScrollRect, Scrollbar

		inline static glm::vec2 GetFontTextSize(const GUIInputFieldComponent& guiIFC) { return s_Instance->GetTextSize(guiIFC.FontAssetHandle, guiIFC.TextStartingOffset, guiIFC.TextScalar, guiIFC.KerningOffset, guiIFC.LineHeightOffset, guiIFC.Text, guiIFC.Padding); }
public:
		virtual ~Renderer() = default;

		virtual const API& GetAPI() = 0;
		virtual Ref<Texture2D> GetWhiteTexture() = 0;
		virtual const glm::mat4& GetIdentityMat4() = 0;
		virtual const float& GetLineWidth() = 0;
		virtual const QuadData::Vertices& GetVertices(const Pivot& pivot) = 0;
		virtual glm::vec2 GetTextSize(UUID fontHandle, const glm::vec2& offset,
			const float& scalar, const float& kerningOffset, const float& lineHeightOffset, const std::string& text, const glm::vec2& padding) = 0;

		virtual const Statistics& GetStats() = 0;
		virtual void ClearStats() = 0;

protected:
		virtual void CreateData() = 0;
		virtual void ClearData() = 0;
		virtual void ClearBuffers() = 0;

		virtual void Start(const Camera*& camera) = 0;
		virtual void End() = 0;
		virtual void Flush() = 0;

		virtual void SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void SetLineWidth(float thickness) = 0;

		virtual void DrawLines(Ref<VertexArray> vertexArray, uint32_t vertexCount) = 0;
		virtual void DrawIndices(Ref<VertexArray> vertexArray, uint32_t indexCount = 0) = 0;

		// Sprite/Quad
		virtual void DrawQuadColor(const glm::mat4& transform, const Pivot& pivot, const glm::vec4& color,  const uint32_t& entityID = -1) = 0;
		virtual void DrawQuadTexture(const glm::mat4& transform, const Pivot& pivot, Ref<Texture2D> texture, const float& tilingFactor,
			const glm::vec4& color,  const uint32_t& entityID = -1) = 0;
		virtual void DrawQuadSubTexture(const glm::mat4& transform, const Pivot& pivot, Ref<SubTexture2D> subTexture, const float& tilingFactor,
			const glm::vec4& tintColor,  const uint32_t& entityID = -1) = 0;

		virtual void DrawSprite(const glm::mat4& transform, const Pivot& pivot, const SpriteRendererComponent& src,  const uint32_t& entityID) = 0;

		// Circle
		virtual void DrawCircle(const glm::mat4& transform, const Pivot& pivot, const float& radius, const float& thickness, const float& fade,
			const glm::vec4& color,  const uint32_t& entityID = -1) = 0;
		virtual void DrawCircle(const glm::mat4& transform, const Pivot& pivot, const CircleRendererComponent& crc, const uint32_t& entityID) = 0;

		// Line/Rectangle
		virtual void DrawRectangle(const glm::mat4& transform, const Pivot& pivot, const glm::vec4& color, const uint32_t& entityID) = 0;
		virtual void DrawLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color,
			const uint32_t& entityID = -1) = 0;

		// Text
		virtual void DrawString(const glm::mat4& transform, const std::string& text, Ref<Font> font,
			const glm::vec4& texColor, const glm::vec4& bgColor, const float& kerningOffset,
			const float& lineHeightOffset, const float& scalar, const glm::vec2& offset, const uint32_t& entityID = -1) = 0;
		virtual void DrawString(const glm::mat4& transform, const TextRendererComponent& trc, const uint32_t& entityID) = 0;

		// GUI
		virtual void DrawButton(const glm::mat4& transform, const Pivot& pivot, const GUIButtonComponent& guiBC, const GUIState& state, const uint32_t& entityID) = 0;
		virtual void DrawInputField(const glm::mat4& transform, const Pivot& pivot, const GUIInputFieldComponent& guiIFC, const GUIState& state, const uint32_t& entityID) = 0;
		virtual void DrawSlider(const glm::mat4& transform, const Pivot& pivot, const GUISliderComponent& guiSC, const GUIState& state, const uint32_t& entityID) = 0;
		virtual void DrawCheckbox(const glm::mat4& transform, const Pivot& pivot, const GUICheckboxComponent& guiCB, const GUIState& state, const uint32_t& entityID) = 0;
	private:

		// Sprite/Quad
		virtual void SetQuadData(const glm::mat4& transform, const Pivot& pivot,
			const uint32_t& textureIndex, const glm::vec2 textureCoords[4], const float& tilingFactor, 
			const glm::vec4& color,  const uint32_t& entityID = -1) = 0;
		virtual void ResetQuadData() = 0;

		// Circle
		virtual void SetCircleData(const glm::mat4& transform, const Pivot& pivot, const float& radius, const float& thickness, const float& fade,
			const glm::vec4& color, const uint32_t& entityID = -1) = 0;
		virtual void ResetCircleData() = 0;

		// Line/Rectangle
		virtual void SetLineData(const glm::vec3& initialPosition, const glm::vec3& finalPosition, 
			const glm::vec4& color, const uint32_t& entityID) = 0;
		virtual void ResetLineData() = 0;

		// Text
		virtual void SetTextData(const glm::mat4& transform, const glm::vec2& minTextureCoord, const glm::vec2& maxTextureCoord,
			const glm::vec2& minQuadPlane, const glm::vec2& maxQuadPlane, Ref<Font> font,
			const glm::vec4& texColor, const glm::vec4& bgColor, const uint32_t& entityID) = 0;
		virtual void ResetTextData() = 0;
		
	private:
		static Ref<Renderer> s_Instance;
	};

}