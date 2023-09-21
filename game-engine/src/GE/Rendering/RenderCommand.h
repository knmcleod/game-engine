#pragma once
#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RenderAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RenderAPI->SetViewport(x, y, width, height);
		}

		// Clears RenderAPI in use
		inline static void Clear()
		{ 
			s_RenderAPI->Clear(); 
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RenderAPI->SetClearColor(color);
		}

		// Draws elements to RenderAPI in use
		inline static void DrawIndices(const Ref<VertexArray>& vertexArray)
		{
			s_RenderAPI->DrawIndices(vertexArray);
		}

	private:
		static RenderAPI* s_RenderAPI;
	};
}
