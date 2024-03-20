#include "GE/GEpch.h"

#include "Application.h"

#include "GE/Core/EntryPoint.h"
#include "GE/Core/Time/Time.h"

#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/VertexArray/VertexArray.h"

#include <GLFW/glfw3.h>

namespace GE
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, s_Instance, std::placeholders::_1)
	
	Application* Application::s_Instance = 0;

	Application::Application(const ApplicationSpecification specification) : m_Specification(specification)
	{
		GE_PROFILE_FUNCTION();
		GE_CORE_INFO("Core Application Constructor Start.");
		GE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// Sets working directory
		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		// Creates Window and Binds Events
		m_Window = Window::Create(WindowProps(specification.Name));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		
		RenderCommand::Init();

		// Creates ImGui Layer
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
		GE_CORE_INFO("Core Application Constructor Complete.");
	}

	Application::~Application()
	{
		GE_CORE_INFO("Core Application Destructor Start.");

		RenderCommand::ShutDown();
		GE_CORE_INFO("Core Application Destructor Complete.");
	}

	void Application::Run()
	{
		GE_PROFILE_FUNCTION();

		while (m_Running)
		{
			Timestep timestep;
			{
				GE_PROFILE_SCOPE("Time Compilation - Application::Run()");
				float time = (float)glfwGetTime();
				timestep = time - m_LastFrameTime;
				m_LastFrameTime = time;
			}

			ExecuteMainThread();

			if (!m_Minimized)
			{
				{ //	Updates Layers
					GE_PROFILE_SCOPE("LayerStack - Application::Run()");
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
				GE_PROFILE_SCOPE("Window - Application::Run()");
				m_Window->OnUpdate();
			}
		}
	}

	void Application::Close()
	{
		GE_CORE_INFO("Application Closed.");
		m_Running = false;
	}

#pragma region Layer Handling

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

		if (m_LayerStack.PopLayer(layer))
			layer->OnDetach();
	}

	void Application::PopOverlay(Layer* overlay)
	{
		GE_PROFILE_FUNCTION();

		if (m_LayerStack.PopOverlay(overlay))
			overlay->OnDetach();
	}
#pragma endregion

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		// Lock for this scope, won't lock again till unlocked
		std::scoped_lock<std::mutex> lock(m_MainThreadMutex);

		m_MainThread.emplace_back(function);
	}

	void Application::ExecuteMainThread()
	{
		std::vector<std::function<void()>> copy;
		{
			// Lock for this scope, won't lock again till unlocked(finished)
			std::scoped_lock<std::mutex> lock(m_MainThreadMutex);
			copy = m_MainThread;
			m_MainThread.clear();
		}

		for (auto& func : copy)
			func();
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

}