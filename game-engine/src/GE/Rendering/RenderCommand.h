#pragma once

#include "GE/Rendering/RenderAPI.h"
#include "GE/Rendering/Renderer/2D/Renderer2D.h"

namespace GE
{
	class RenderCommand
	{
		friend class Renderer2D;
	public:
		/*
		* Initializes RenderAPI & Renderer2D
		*/
		inline static void Init()
		{
			s_RenderAPI->Init();
			Renderer2D::Init();
		}

		/*
		* Shutdowns RenderAPI & Renderer2D
		*/
		inline static void ShutDown()
		{
			Renderer2D::ShutDown();
			ClearAPI();
		}

		/*
		* Clears RenderAPI in use
		* Renderer2D stays active
		*/
		inline static void ClearAPI()
		{
			s_RenderAPI->Clear();
		}

		inline static void SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height)
		{
			s_RenderAPI->SetViewport(xOffset, yOffset, width, height);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RenderAPI->SetClearColor(color);
		}

		// Sets lines width to RenderAPI in use
		inline static void SetLineWidth(float thickness)
		{
			s_RenderAPI->SetLineWidth(thickness);
		}

	private:
		/*
		* Draws lines to RenderAPI in use
		* Use Renderer2D to produce VertexArray
		*/
		inline static void DrawLines(Ref<VertexArray> vertexArray, uint32_t vertexCount)
		{
			s_RenderAPI->DrawLines(vertexArray, vertexCount);
		}

		/*
		* Draws indices to RenderAPI in use
		* Use Renderer2D to produce VertexArray
		*/
		inline static void DrawIndices(Ref<VertexArray> vertexArray, uint32_t indexCount)
		{
			s_RenderAPI->DrawIndices(vertexArray, indexCount);
		}
	private:
		static RenderAPI* s_RenderAPI;
	};
}
