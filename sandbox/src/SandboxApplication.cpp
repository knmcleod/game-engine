#include <GE/GE.h>

#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public GE::Layer
{
public:
	ExampleLayer() : GE::Layer("Example"), m_OrthoCamera(-1.0f, 1.0f, -1.0f, 1.0f)
	{
		//Creates Vertex Array
		m_VertexArray.reset(GE::VertexArray::Create());

		//Creates Vertex Buffer
		float vertices[] = {
			// positions		// colors			// texture coords
			0.5f, 0.5f, 0.0f,	1.0f, 0.0f, 0.0f,	1.0f, 1.0f,			// top right
			0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,			// bottom right
			-0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 0.0f,	0.0f, 0.0f,			// bottom left
			-0.5f, 0.5f, 0.0f,	1.0f, 1.0f, 0.0f,	0.0f, 1.0f			// top left
		};
		GE::Ref<GE::VertexBuffer> m_VertexBuffer;
		m_VertexBuffer.reset(GE::VertexBuffer::Create(sizeof(vertices), vertices));

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
		uint32_t indices[] = { 0, 1, 2 };
		GE::Ref<GE::IndexBuffer> m_IndexBuffer;
		m_IndexBuffer.reset(GE::IndexBuffer::Create(std::size(indices), indices));

		//Add Index Buffer to Vertex Array
		m_VertexArray->AddIndexBuffer(m_IndexBuffer);

		//Creates TextureShader
		std::string vertexTextureSrc = R"(#version 330 core
			layout(location = 0) in vec3 a_Transform;
			layout(location = 1) in vec3 a_Color;
			layout(location = 2) in vec2 a_TextureCoord;

			out vec3 v_Color;
			out vec2 v_TextureCoord;
			
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Transform, 1.0);
				v_Color = a_Color;
				v_TextureCoord = a_TextureCoord;
			}
		)";

		std::string fragmentTextureSrc = R"(#version 330 core
			in vec3 v_Color;
			in vec2 v_TextureCoord;

			out vec4 color;

			uniform sampler2D u_Texture;

			void main()
			{
				color = texture2D(u_Texture, v_TextureCoord) * vec4(v_Color, 1.0);
			}
		)";

		m_TextureShader.reset(GE::Shader::Create(vertexTextureSrc, fragmentTextureSrc));
		m_Texture.reset(GE::Texture2D::Create("assets/textures/image.jpg"));

		m_TextureShader->Bind();
		m_Texture->Bind();

		m_TextureShader->UploadUniformInt("u_Texture", 0);

	}

	virtual void OnUpdate(GE::Timestep timestep) override
	{
		//	Camera Movement
		if (GE::Input::IsKeyPressed(GE_KEY_LEFT))
		{
			m_CameraPosition.x -= m_CameraMoveSpeed * timestep;
		}
		else if (GE::Input::IsKeyPressed(GE_KEY_RIGHT))
		{
			m_CameraPosition.x += m_CameraMoveSpeed * timestep;
		}

		if (GE::Input::IsKeyPressed(GE_KEY_UP))
		{
			m_CameraPosition.y += m_CameraMoveSpeed * timestep;
		}
		else if (GE::Input::IsKeyPressed(GE_KEY_DOWN))
		{
			m_CameraPosition.y -= m_CameraMoveSpeed * timestep;
		}

		if (GE::Input::IsKeyPressed(GE_KEY_A))
		{
			m_CameraRotation -= m_CameraRotationSpeed * timestep;
		}
		else if (GE::Input::IsKeyPressed(GE_KEY_D))
		{
			m_CameraRotation += m_CameraRotationSpeed * timestep;
		}

		//	Shader Movement
		if (GE::Input::IsKeyPressed(GE_KEY_Q))
		{
			m_ShaderPosition.x -= m_ShaderMoveSpeed * timestep;
		}
		else if (GE::Input::IsKeyPressed(GE_KEY_E))
		{
			m_ShaderPosition.x += m_ShaderMoveSpeed * timestep;
		}

		if (GE::Input::IsKeyPressed(GE_KEY_W))
		{
			m_ShaderPosition.y += m_ShaderMoveSpeed * timestep;
		}
		else if (GE::Input::IsKeyPressed(GE_KEY_S))
		{
			m_ShaderPosition.y -= m_ShaderMoveSpeed * timestep;
		}

		GE::RenderCommand::SetClearColor({ 0.5f, 0.5f, 0.5f, 1.0f });
		GE::RenderCommand::Clear();

		m_OrthoCamera.SetPosition(m_CameraPosition);
		m_OrthoCamera.SetRotation(m_CameraRotation);

		GE::Renderer::Start(m_OrthoCamera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		glm::mat4 transform = glm::translate(glm::mat4(1.0), m_ShaderPosition) * scale;

		GE::Renderer::Run(m_TextureShader, m_VertexArray, transform);

		GE::Renderer::End();
	}

private:
	//	Rendering Variables
	GE::Ref<GE::VertexArray> m_VertexArray;
	GE::Ref<GE::Shader> m_TextureShader;
	GE::Ref<GE::Texture> m_Texture;

	GE::OrthographicCamera m_OrthoCamera;

	glm::vec3 m_CameraPosition = glm::vec3(0.0f);
	float m_CameraMoveSpeed = 0.25f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 0.25;

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
