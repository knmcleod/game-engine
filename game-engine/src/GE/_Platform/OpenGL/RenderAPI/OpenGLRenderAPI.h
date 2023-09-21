#pragma once

#include <glad/glad.h>

#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	class OpenGLRenderAPI : public RenderAPI
	{
	public:
		virtual void Init() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		//	Clears Buffers using OpenGL
		virtual void Clear() override;

		virtual void SetClearColor(const glm::vec4& color) override;
		
		//	Draws elements using OpenGL
		virtual void DrawIndices(const Ref<VertexArray>& vertexArray) override;
	};
}

