#pragma once

#include <glad/glad.h>

#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	class OpenGLRenderAPI : public RenderAPI
	{
	public:
		virtual void Clear() override;
		virtual void DrawIndices(const std::shared_ptr<VertexArray>& vertexArray) override;
	};
}

