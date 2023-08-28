#include "GEpch.h"

#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	void Renderer::Start()
	{

	}

	void Renderer::End()
	{

	}

	void Renderer::Run(std::shared_ptr<VertexArray>& vertexArray)
	{
		vertexArray->Bind();
		RenderCommand::DrawIndices(vertexArray);
	}
}
