#include "GE/GEpch.h"

#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	Renderer::Data* Renderer::m_Data = new Renderer::Data;

	void Renderer::Init()
	{
		RenderCommand::Init();

	}

	void Renderer::ShutDown()
	{

		RenderCommand::ShutDown();
	}

	void Renderer::Start(OrthographicCamera& orthoCamera)
	{
		m_Data->ViewProjectionMatrix = orthoCamera.GetViewProjectionMatrix();
	}

	void Renderer::End()
	{

	}

	void Renderer::Run(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray,
		const glm::mat4& transform)
	{
		shader->Bind();
		shader->SetMat4("u_ViewProjection", m_Data->ViewProjectionMatrix);
		shader->SetMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndices(vertexArray);
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}
}
