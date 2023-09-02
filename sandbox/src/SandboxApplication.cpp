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
		float vertices[3 * 4] = { -0.5f, -0.5f, 0.0f,
									0.5f, -0.5f, 0.0f,
									0.0f, 0.5f, 0.0f,
									0.0f, 0.0f, 0.0f};
		std::shared_ptr<GE::VertexBuffer> m_VertexBuffer;
		m_VertexBuffer.reset(GE::VertexBuffer::Create(sizeof(vertices), vertices));

		//Sets up Layout using Vertex Buffer
		GE::BufferLayout layout =
		{
			{ GE::Shader::ShaderDataType::Float3, "a_Position" }
		};
		m_VertexBuffer->SetLayout(layout);

		//Add Vertex Buffer to Vertex Array
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		//Creates Index Buffer
		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<GE::IndexBuffer> m_IndexBuffer;
		m_IndexBuffer.reset(GE::IndexBuffer::Create(std::size(indices), indices));

		//Add Index Buffer to Vertex Array
		m_VertexArray->AddIndexBuffer(m_IndexBuffer);

		//Creates Shader
		std::string vertexSrc = R"(#version 330 core
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(#version 330 core
			layout(location = 0) out vec4 a_color;

			in vec3 v_Position;
			in vec4 v_Color;
			
			uniform vec3 u_Color;

			void main()
			{
				a_color = vec4(u_Color, 1.0f);
			}
		)";

		m_Shader.reset(GE::Shader::Create(vertexSrc, fragmentSrc));
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

		GE::RenderCommand::Clear();

		m_OrthoCamera.SetPosition(m_CameraPosition);
		m_OrthoCamera.SetRotation(m_CameraRotation);

		GE::Renderer::Start(m_OrthoCamera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

		glm::mat4 transform = glm::translate(glm::mat4(1.0), m_ShaderPosition) * scale;

		m_Shader->UploadUniformFloat3("u_Color", m_shaderColor);

		GE::Renderer::Run(m_Shader, m_VertexArray, transform);

		GE::Renderer::End();
	}

private:
	//	Rendering Variables
	std::shared_ptr<GE::Shader> m_Shader;
	std::shared_ptr<GE::VertexArray> m_VertexArray;

	GE::OrthographicCamera m_OrthoCamera;

	glm::vec3 m_CameraPosition = glm::vec3(0.0f);
	float m_CameraMoveSpeed = 0.1f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 0.1;

	glm::vec3 m_ShaderPosition = glm::vec3(1.0f);
	float m_ShaderMoveSpeed = 0.5f;

	glm::vec3 m_shaderColor = { 0.8f, 0.2f, 0.5f};
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
