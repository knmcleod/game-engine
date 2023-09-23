#include "GE/GEpch.h"

#include "Renderer2D.h"

namespace GE
{
	struct Renderer2DData
	{
		Ref<VertexArray> VertexArray;
		Ref<Shader> Shader;
		Ref<Texture2D> Texture;
	};

	static Renderer2DData* s_Data;

	void Renderer2D::Init()
	{
		RenderCommand::Init();

		s_Data = new Renderer2DData();

		//Creates Vertex Array
		s_Data->VertexArray = VertexArray::Create();

		//Creates Vertex Buffer
		/*float vertices[] = {
			// Transform		// TextureCoord
			0.5f, 0.5f, 0.0f,	1.0f, 1.0f,		// top right
			0.5f, -0.5f, 0.0f,	1.0f, 0.0f,		// bottom right
			-0.5f, -0.5f, 0.0f,	0.0f, 0.0f,		// bottom left
			-0.5f, 0.5f, 0.0f,	0.0f, 1.0f		// top left
		};*/
		float vertices[] = {
			// positions		// colors					// texture coords
			0.5f, 0.5f, 0.0f,	1.0f, 0.0f, 0.0f, 0.0f,		1.0f, 1.0f,			// top right
			0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f, 0.0f,		1.0f, 0.0f,			// bottom right
			-0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 0.0f, 0.0f,		0.0f, 0.0f,			// bottom left
			-0.5f, 0.5f, 0.0f,	1.0f, 1.0f, 0.0f, 0.0f,		0.0f, 1.0f			// top left
		};
		Ref<VertexBuffer> m_VertexBuffer;
		m_VertexBuffer = VertexBuffer::Create(sizeof(vertices), vertices);

		//Sets up Layout using Vertex Buffer
		BufferLayout layout =
		{
			{ GE::Shader::ShaderDataType::Float3, "a_Transform" },
			{ GE::Shader::ShaderDataType::Float4, "a_Color" },
			{ GE::Shader::ShaderDataType::Float2, "a_TextureCoord" }
		};
		m_VertexBuffer->SetLayout(layout);

		//Add Vertex Buffer to Vertex Array
		s_Data->VertexArray->AddVertexBuffer(m_VertexBuffer);

		//Creates Index Buffer
		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> m_IndexBuffer;
		m_IndexBuffer = IndexBuffer::Create(static_cast<unsigned int>(std::size(indices)), indices);

		//Add Index Buffer to Vertex Array
		s_Data->VertexArray->AddIndexBuffer(m_IndexBuffer);

		uint32_t textureData = 0xFFFFFFFF;
		s_Data->Texture = Texture2D::Create(1, 1, 4, &textureData, sizeof(uint32_t));
		//s_Data->Texture->SetData(&textureData, sizeof(uint32_t));

		s_Data->Shader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data->Shader->Bind();
		s_Data->Shader->SetInt("u_Texture", 0);

	}

	void Renderer2D::ShutDown()
	{
		delete s_Data;
		RenderCommand::ShutDown();
	}

	void Renderer2D::Start(const OrthographicCamera& orthoCamera)
	{
		s_Data->Shader->SetMat4("u_ViewProjection", orthoCamera.GetViewProjectionMatrix());
	}

	void Renderer2D::End()
	{
	}

	void Renderer2D::FillQuadColor(const glm::vec3& position, const glm::vec2& size,
		const float rotation, const glm::vec4& color)
	{
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		glm::mat4 transform = glm::translate(glm::mat4(1.0), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1)) * scale;

		s_Data->Shader->SetMat4("u_Transform", transform);
		s_Data->Shader->SetFloat4("u_Color", color);

		s_Data->Texture->Bind();
		
		s_Data->VertexArray->Bind();
		RenderCommand::DrawIndices(s_Data->VertexArray);
	}

	void Renderer2D::FillQuadTexture(const glm::vec3& position, const glm::vec2& size,
		const float rotation, const Ref<Texture> texture, const glm::vec4& color)
	{
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		glm::mat4 transform = glm::translate(glm::mat4(1.0), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1)) * scale;

		s_Data->Shader->SetMat4("u_Transform", transform);
		s_Data->Shader->SetFloat4("u_Color", color);
		
		texture->Bind();

		s_Data->VertexArray->Bind();
		RenderCommand::DrawIndices(s_Data->VertexArray);
	}
}
