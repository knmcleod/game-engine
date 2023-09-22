#include <GE/GE.h>

#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public GE::Layer
{
public:
	ExampleLayer() : GE::Layer("Example"), m_OrthoCameraController(1280.0f/720.0f, true)
	{
		//Creates Vertex Array
		m_VertexArray = GE::VertexArray::Create();

		//Creates Vertex Buffer
		float vertices[] = {
			// positions		// colors			// texture coords
			0.5f, 0.5f, 0.0f,	1.0f, 0.0f, 0.0f,	1.0f, 1.0f,			// top right
			0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,			// bottom right
			-0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 0.0f,	0.0f, 0.0f,			// bottom left
			-0.5f, 0.5f, 0.0f,	1.0f, 1.0f, 0.0f,	0.0f, 1.0f			// top left
		};
		GE::Ref<GE::VertexBuffer> m_VertexBuffer;
		m_VertexBuffer = GE::VertexBuffer::Create(sizeof(vertices), vertices);

		//Sets up Layout using Vertex Buffer
		GE::BufferLayout layout =
		{
			{ GE::Shader::ShaderDataType::Float3, "a_Transform" },
			{ GE::Shader::ShaderDataType::Float3, "a_Color" },
			{ GE::Shader::ShaderDataType::Float2, "a_TextureCoord" }
		};
		m_VertexBuffer->SetLayout(layout);

		//Add Vertex Buffer to Vertex Array
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		//Creates Index Buffer
		uint32_t indices[] = { 0, 1, 2, 3 };
		GE::Ref<GE::IndexBuffer> m_IndexBuffer;
		m_IndexBuffer = GE::IndexBuffer::Create(static_cast<unsigned int>(std::size(indices)), indices);

		//Add Index Buffer to Vertex Array
		m_VertexArray->AddIndexBuffer(m_IndexBuffer);

		m_Texture = GE::Texture2D::Create("assets/textures/image.jpg");
		m_Texture->Bind();

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl", "textureShader");
		textureShader->Bind();
		textureShader->UploadUniformInt("u_Texture", 0);
	}

	virtual void OnUpdate(GE::Timestep timestep) override
	{
		m_OrthoCameraController.OnUpdate(timestep);

		//	Shader Movement
		if (GE::Input::IsKeyPressed(GE_KEY_LEFT))
		{
			m_ShaderPosition.x -= m_ShaderMoveSpeed * timestep;
		}
		else if (GE::Input::IsKeyPressed(GE_KEY_RIGHT))
		{
			m_ShaderPosition.x += m_ShaderMoveSpeed * timestep;
		}

		if (GE::Input::IsKeyPressed(GE_KEY_UP))
		{
			m_ShaderPosition.y += m_ShaderMoveSpeed * timestep;
		}
		else if (GE::Input::IsKeyPressed(GE_KEY_DOWN))
		{
			m_ShaderPosition.y -= m_ShaderMoveSpeed * timestep;
		}

		GE::RenderCommand::SetClearColor({ 0.5f, 0.5f, 0.5f, 1.0f });
		GE::RenderCommand::Clear();

		GE::Renderer::Start(m_OrthoCameraController.GetCamera());
		//GE::Renderer2D::Start(m_OrthoCamera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		glm::mat4 transform = glm::translate(glm::mat4(1.0), m_ShaderPosition) * scale;

		auto textureShader = m_ShaderLibrary.Get("textureShader");
		GE::Renderer::Run(textureShader, m_VertexArray, transform);

		GE::Renderer::End();
	}

	virtual void OnEvent(GE::Event& e)
	{
		m_OrthoCameraController.OnEvent(e);
	}

private:
	GE::ShaderLibrary m_ShaderLibrary;

	GE::Ref<GE::VertexArray> m_VertexArray;
	GE::Ref<GE::Texture> m_Texture;

	GE::OrthographicCameraController m_OrthoCameraController;

	glm::vec3 m_ShaderPosition = glm::vec3(0.0f);
	float m_ShaderMoveSpeed = 0.5f;

};

class Sandbox : public GE::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}

private:

};

GE::Application* GE::CreateApplication()
{
	return new Sandbox();
}
