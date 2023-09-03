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

	void Renderer::Run(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		shader->UploadUniformMat4("u_ViewProjection", m_Data->ViewProjectionMatrix);
		shader->UploadUniformMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndices(vertexArray);
	}
}
