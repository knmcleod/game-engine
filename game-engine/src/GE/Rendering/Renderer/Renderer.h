#pragma once

#include "GE/Rendering/RenderCommand.h"

namespace GE
{
	class Renderer
	{
	public:
		inline static RenderAPI::API GetAPI() { return RenderAPI::GetAPI(); }

		static void Start();
		static void Run(std::shared_ptr<VertexArray>& vertexArray);
		static void End();

	};
}

