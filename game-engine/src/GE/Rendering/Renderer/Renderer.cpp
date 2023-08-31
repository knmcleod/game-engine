#include "GEpch.h"

#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	Renderer::Data* Renderer::m_Data = new Renderer::Data;

	void Renderer::Start(OrthographicCamera& orthoCamera)
	{
		m_Data->ViewProjectionMatrix = orthoCamera.GetViewProjectionMatrix();
	}

	void Renderer::End()
	{

	}

	void Renderer::Run(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray)
	{
		shader->Bind();
		shader->UploadUniformMat4("u_ViewProjection", m_Data->ViewProjectionMatrix);

		vertexArray->Bind();
		RenderCommand::DrawIndices(vertexArray);
	}
}
