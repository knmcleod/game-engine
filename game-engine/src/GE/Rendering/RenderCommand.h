#pragma once
#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	class RenderCommand
	{
	public:
		// Clears RenderAPI in use
		inline static void Clear()
		{ 
			s_RenderAPI->Clear(); 
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
