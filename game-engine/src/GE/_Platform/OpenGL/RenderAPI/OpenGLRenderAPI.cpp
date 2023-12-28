#include "GE/GEpch.h"

#include "OpenGLRenderAPI.h"

namespace GE
{
	void OpenGLRenderAPI::Init()
	{
		glEnable(GL_DEPTH);
		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC0_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	void OpenGLRenderAPI::SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height)
	{
		glViewport(xOffset, yOffset, width, height);
	}

	void OpenGLRenderAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRenderAPI::DrawIndices(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		
	}

	void OpenGLRenderAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);

	}

	void OpenGLRenderAPI::SetLineWidth(float thickness)
	{
		glLineWidth(thickness);
	}
}