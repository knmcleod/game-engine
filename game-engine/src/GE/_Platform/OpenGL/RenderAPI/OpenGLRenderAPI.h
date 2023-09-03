#pragma once

#include <glad/glad.h>

#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	class OpenGLRenderAPI : public RenderAPI
	{
	public:
		//	Clears Buffers using OpenGL
		virtual void Clear() override;

		//	Draws elements using OpenGL
		virtual void DrawIndices(const Ref<VertexArray>& vertexArray) override;
	};
}

