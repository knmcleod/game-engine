#pragma once

#include <glad/glad.h>

#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	class OpenGLRenderAPI : public RenderAPI
	{
	public:
		virtual void Init() override;

		//	Clears Buffers using OpenGL
		virtual void Clear() override;

		virtual void SetClearColor(const glm::vec4& color) override;
		
		//	Draws elements using OpenGL
		virtual void DrawIndices(const Ref<VertexArray>& vertexArray) override;
	};
}

