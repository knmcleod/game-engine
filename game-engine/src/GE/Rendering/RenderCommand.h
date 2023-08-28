#pragma once
#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	class RenderCommand
	{
	public:
		inline static void Clear()
		{ 
			s_RenderAPI->Clear(); 
		}

		inline static void DrawIndices(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RenderAPI->DrawIndices(vertexArray);
		}
	private:
		static RenderAPI* s_RenderAPI;
	};
}
