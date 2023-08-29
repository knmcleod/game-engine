#include "GEpch.h"

#include "GE/_Platform/OpenGL/RenderAPI/OpenGLRenderAPI.h"

namespace GE
{
	void OpenGLRenderAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderAPI::DrawIndices(std::shared_ptr<VertexArray> const& vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

	}
}