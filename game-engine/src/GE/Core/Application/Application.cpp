#include "GE/GEpch.h"

#include "Application.h"

#include "GE/Asset/RuntimeAssetManager.h"

#include "GE/Core/FileSystem/FileSystem.h"
#include "GE/Core/Time/Timestep.h"

#include "GE/Project/Project.h"

#include "GE/Rendering/RenderCommand.h"

#include "GE/Scripting/Scripting.h"

namespace GE
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, s_Instance, std::placeholders::_1)
	
	Application* Application::s_Instance = 0;

	Application::Application(const Application::Config& spec) : p_Config(spec)
	{
		GE_PROFILE_FUNCTION();
		GE_CORE_INFO("Core Application Constructor Start.");
		GE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		//  Sets working directory
		if (!p_Config.WorkingDirectory.empty())
			std::filesystem::current_path(p_Config.WorkingDirectory);
		
		if (!LoadProject())
		{
			GE_CORE_ERROR("Could not Load Application Project.");
			Close();
			return;
		}

		Project::NewAssetManager<RuntimeAssetManager>();

		// Creates Window and Binds Events
		p_Window = Window::Create(Window::Config(p_Config.Name, Project::GetWidth(), Project::GetHeight(), true, nullptr));
		p_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		
		RenderCommand::Init();

		p_ImGuiLayer = new ImGuiLayer();
		PushOverlay(p_ImGuiLayer);

		GE_CORE_INFO("Core Application Constructor Complete.");
	}

	Application::~Application()
	{
		GE_CORE_INFO("Core Application Destructor Start.");
		
		RenderCommand::ShutDown();
		Project::Shutdown();
		
		GE_CORE_INFO("Core Application Destructor Complete.");
	}

	const uint32_t& Application::GetWidth() const
	{
		return Project::GetWidth();
	}

	const uint32_t& Application::GetHeight() const
	{
		return Project::GetHeight();
	}

	void Application::Run()
	{
		GE_PROFILE_FUNCTION();

		while (p_Running)
		{
			{
				GE_PROFILE_SCOPE("Time Compilation - Application::Run()");
				float time = p_Window->GetTime();
				p_TS = time - p_LastFrameTime;
				p_LastFrameTime = time;
			}

			ExecuteMainThread();

			if (!p_Minimized)
			{
				{ //	Updates Layers
					GE_PROFILE_SCOPE("LayerStack - Application::Run()");
					for (Layer* layer : p_LayerStack)
					{
						layer->OnUpdate(p_TS);
					}
				}
				{ //	Updates ImGui
					GE_PROFILE_SCOPE("ImGui LayerStack - Application::Run()");
					p_ImGuiLayer->Begin();
					for (Layer* layer : p_LayerStack)
					{
						layer->OnImGuiRender();
					}
					p_ImGuiLayer->End();
				}
			}

			{ //	Updates Window
				GE_PROFILE_SCOPE("Window - Application::Run()");
				p_Window->OnUpdate();
			}
		}
	}

	void Application::Close()
	{
		p_Running = false;
		GE_CORE_INFO("Application Closed.");
	}

#pragma region Layer Handling

	void Application::PushLayer(Layer* layer)
	{
		GE_PROFILE_FUNCTION();

		p_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		GE_PROFILE_FUNCTION();

		p_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		GE_PROFILE_FUNCTION();

		if (p_LayerStack.PopLayer(layer))
			layer->OnDetach();
	}

	void Application::PopOverlay(Layer* overlay)
	{
		GE_PROFILE_FUNCTION();

		if (p_LayerStack.PopOverlay(overlay))
			overlay->OnDetach();
	}
#pragma endregion

#pragma region Thread Handling

	void Application::SubmitToMainThread(const std::function<void()>& func)
	{
		// Lock for this scope, won't lock again till unlocked
		std::scoped_lock<std::mutex> lock(p_MainThreadMutex);

		p_MainThread.emplace_back(func);
	}

	void Application::ExecuteMainThread()
	{
		std::vector<std::function<void()>> copy;
		{
			// Lock for this scope, won't lock again till unlocked(finished)
			std::scoped_lock<std::mutex> lock(p_MainThreadMutex);
			copy = p_MainThread;
			p_MainThread.clear();
		}

		for (auto& func : copy)
			func();
	}

#pragma endregion

#pragma region Event Handling

	void Application::OnEvent(Event& e)
	{
		GE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = p_LayerStack.end(); it != p_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.IsHandled())
				break;
		}

	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		GE_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			p_Minimized = true;
			return false;
		}

		//RenderCommand::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
		Project::SetViewport(e.GetWidth(), e.GetHeight());

		p_Minimized = false;
		return false;
	}

#pragma endregion

#pragma region Project Handling
	bool Application::LoadProject() const
	{
		if (p_Config.Args.Count < 1)
		{
			GE_CORE_WARN("Application::LoadProject - Project file path not found at index 1. \n\tTrying Load from File Dialog.");
			return LoadProjectFileDialog();
		}

		return LoadProject(p_Config.Args[1]);
	}

	bool Application::LoadProject(const std::filesystem::path& path) const
	{
		return Project::Load(path);
	}

	bool Application::LoadProjectFileDialog() const
	{
		std::string filePath = FileSystem::LoadFromFileDialog("GAME Project(*.gproj)\0*.gproj\0");
		if (filePath.empty())
			return false;

		return LoadProject(filePath);
	}

	bool Application::SaveProject() const
	{
		const std::filesystem::path& path = Project::GetProjectPath() / std::filesystem::path(Project::GetConfig().Name + ".gproj");
		if (!path.empty())
		{
			return SaveProject(path);
		}
		else
			return SaveProjectFileDialog();
	}

	bool Application::SaveProject(const std::filesystem::path& path) const
	{
		path.extension() = ".gproj";
		return Project::Save(path);
	}

	bool Application::SaveProjectFileDialog() const
	{
		std::string filePath = FileSystem::SaveFromFileDialog("GAME Project(*.gproj)\0 * .gproj\0");
		if (filePath.empty())
			return false;

		return SaveProject(filePath);
	}

#pragma endregion


}