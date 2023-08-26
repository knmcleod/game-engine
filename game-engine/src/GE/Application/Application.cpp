#include "GEpch.h"
#include "Application.h"

namespace GE
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
	
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		GE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		Renderer::SetAPI(Renderer::RendererAPI::OpenGL);

		//Creates Window and Binds Events
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		
		//Creates ImGui Layer
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		//Creates Vertex Array
		m_VertexArray.reset(VertexArray::Create());

		//Creates Vertex Buffer
		float vertices[3 * 3] = { -0.5f, -0.5f, 0.0f,
									0.5f, -0.5f, 0.0f,
									0.0f, 0.5f, 0.0f };
		m_VertexBuffer.reset(VertexBuffer::Create(sizeof(vertices), vertices));
		
		//Sets up Layout using Vertex Buffer
		BufferLayout layout =
		{
			{ Shader::ShaderDataType::Float3, "a_Position" },
			{ Shader::ShaderDataType::Float4, "a_Color" }
		};
		m_VertexBuffer->SetLayout(layout);

		//Add Vertex Buffer to Vertex Array
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		//Creates Index Buffer
		uint32_t indices[3] = { 0, 1, 2 };
		m_IndexBuffer.reset(IndexBuffer::Create(std::size(indices), indices));
		
		//Add Index Buffer to Vertex Array
		m_VertexArray->AddIndexBuffer(m_IndexBuffer);

		//Creates Shader
		std::string vertexSrc = R"(#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
out vec3 v_Position;
out vec4 v_Color;
void main()
{
v_Position = a_Position;
v_Color = a_Color;
gl_Position = vec4(a_Position, 1.0);
}
)";

		std::string fragmentSrc = R"(#version 330 core
layout(location = 0) out vec4 a_color;
in vec3 v_Position;
void main()
{
a_color = vec4(v_Position * 0.5 + 0.5, 1.0);
})";
		m_Shader.reset(Shader::Create(vertexSrc, fragmentSrc));
	}

	Application::~Application()
	{

	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	void Application::Run()
	{
		while (m_Running)
		{

			m_Shader->Bind();
			m_VertexArray->Bind();
			
			//	Updates Layers
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}

			//	Updates ImGui
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
			{
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
			//

			//	Updates Window
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}

	}

	//	Layer Handling
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}
}