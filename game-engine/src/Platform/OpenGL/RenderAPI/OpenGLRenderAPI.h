#pragma once

#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	class OpenGLRenderAPI : public RenderAPI
	{
	public:
		virtual void Init() override;
		virtual void Clear() override;

		virtual void SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void SetLineWidth(float thickness) override;

		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		virtual void DrawIndices(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
	};
}

