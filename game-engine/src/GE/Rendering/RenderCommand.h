#pragma once
#include "GE/Rendering/RenderAPI.h"
#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RenderAPI->Init();
			Renderer::Init();
		}

		inline static void ShutDown()
		{
			Renderer::ShutDown();
			ClearAPI();
		}

		inline static void SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height)
		{
			s_RenderAPI->SetViewport(xOffset, yOffset, width, height);
		}

		// Clears RenderAPI in use
		inline static void ClearAPI()
		{ 
			s_RenderAPI->Clear(); 
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RenderAPI->SetClearColor(color);
		}

		// Draws elements to RenderAPI in use
		inline static void DrawIndices(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RenderAPI->DrawIndices(vertexArray, indexCount);
		}

	private:
		static RenderAPI* s_RenderAPI;
	};
}
