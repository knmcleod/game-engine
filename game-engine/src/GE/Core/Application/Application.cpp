#include "GE/GEpch.h"

#include "GE/Core/Application/Application.h"

#include "GE/Rendering/RenderCommand.h"

#include <GLFW/glfw3.h>

namespace GE
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, s_Instance, std::placeholders::_1)
	
	Application* Application::s_Instance = 0;

	Application::Application()
	{
		GE_PROFILE_FUNCTION();

		GE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		//Creates Window and Binds Events
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		
		RenderCommand::Init();

		//Creates ImGui Layer
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		RenderCommand::ShutDown();
	}

	void Application::Run()
	{
		GE_PROFILE_FUNCTION();

		while (m_Running)
		{
			Timestep timestep;
			{
				GE_PROFILE_SCOPE("Time Compilation - Application::Run()");
				float time = (float)glfwGetTime(); //Platform::GetTime();
				timestep = time - m_LastFrameTime;
				m_LastFrameTime = time;
			}

			if (!m_Minimized)
			{
				{ //	Updates Layers
					GE_PROFILE_SCOPE("LayerStack Update - Application::Run()");
					for (Layer* layer : m_LayerStack)
					{
						layer->OnUpdate(timestep);
					}
				}
				{ //	Updates ImGui
					GE_PROFILE_SCOPE("ImGui LayerStack - Application::Run()");
					m_ImGuiLayer->Begin();
					for (Layer* layer : m_LayerStack)
					{
						layer->OnImGuiRender();
					}
					m_ImGuiLayer->End();
				}
			}

			{ //	Updates Window
				GE_PROFILE_SCOPE("Window Update - Application::Run()");
				m_Window->OnUpdate();
			}
		}
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		GE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}

	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		this->Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		GE_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		RenderCommand::SetViewport(0, 0, e.GetWidth(), e.GetHeight());

		m_Minimized = false;
		return false;
	}

	//	Layer Handling
	void Application::PushLayer(Layer* layer)
	{
		GE_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		GE_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		GE_PROFILE_FUNCTION();

		if(m_LayerStack.PopLayer(layer))
			layer->OnDetach();
	}

	void Application::PopOverlay(Layer* overlay)
	{
		GE_PROFILE_FUNCTION();

		if(m_LayerStack.PopOverlay(overlay))
			overlay->OnDetach();
	}
}