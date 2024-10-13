#include "GE/GEpch.h"

#include "Application.h"

#include "GE/Asset/RuntimeAssetManager.h"

#include "GE/Audio/AudioManager.h"

#include "GE/Core/FileSystem/FileSystem.h"
#include "GE/Core/Input/Input.h"
#include "GE/Core/Time/Timestep.h"

#include "GE/Project/Project.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "GE/Scripting/Scripting.h"

namespace GE
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, s_Instance, std::placeholders::_1)
	
	Application* Application::s_Instance = 0;

	Entity Application::GetHoveredEntity(float x, float y, int modifier)
	{
		// Calculate mouse position relative to Framebuffer Viewport/Bounds
		glm::vec2 relativeMouse = GetRelativeMouse(x, y, modifier);

		// AttachmentIndex = 1(RED_INTEGER) = entityID
		int entityID = s_Instance->GetHovered(Framebuffer::Attachment::RED_INTEGER, (uint32_t)relativeMouse.x, (uint32_t)relativeMouse.y);
		if (entityID != -1 && Project::GetRuntimeScene())
			return Entity(entityID, Project::GetRuntimeScene().get());
		return {};
	}

	const glm::vec2 Application::GetRelativeMouse(float x, float y, int modifier)
	{
		glm::vec2 relativeMouse = glm::vec2(x, y);
		if (modifier != -1 && modifier != 1)
		{
			GE_CORE_WARN("Unsupported Framebuffer RelativeMouse Modifier. Returning unchanged mouse position.");
			return relativeMouse;
		}

		if (Ref<Framebuffer> framebuffer = s_Instance->GetFramebuffer())
		{
			glm::vec2 modifierMinBounds = glm::vec2(modifier * framebuffer->GetMinBounds().x, modifier * framebuffer->GetMinBounds().y);
			relativeMouse = glm::vec2({x + modifierMinBounds.x}, {y + modifierMinBounds.y });
			glm::vec2 viewportSize = framebuffer->GetMaxBounds() - framebuffer->GetMinBounds();
			relativeMouse.y = viewportSize.y - relativeMouse.y;
		}

		return relativeMouse;
	}

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

		AudioManager::Init();
		Scripting::Init();
		Project::NewAssetManager<RuntimeAssetManager>();

		// Creates Window and Binds Events
		p_Window = Window::Create(Window::Config(p_Config.Name, Project::GetWidth(), Project::GetHeight(), true, nullptr));
		p_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Create();

		{
			GE_PROFILE_SCOPE("Application() - Framebuffer Setup");
			Framebuffer::Config framebufferConfig = Framebuffer::Config(Project::GetWidth(), Project::GetHeight(),
				{	Framebuffer::Attachment::RGBA8,				// 0
					Framebuffer::Attachment::RED_INTEGER,		// 1
					Framebuffer::Attachment::GREEN_INTEGER,		// 2 
					Framebuffer::Attachment::BLUE_INTEGER,		// 3
					Framebuffer::Attachment::DEPTH24STENCIL8	// 4
				});
			p_Framebuffer = Framebuffer::Create(framebufferConfig);
		}

		p_LayerStack = CreateRef<LayerStack>();

		GE_CORE_INFO("Core Application Constructor Complete.");
	}

	Application::~Application()
	{
		GE_CORE_INFO("Core Application Destructor Start.");
		
		Scripting::Shutdown();
		AudioManager::Shutdown();
		Project::Shutdown();
		
		GE_CORE_INFO("Core Application Destructor Complete.");
	}

	void Application::Run()
	{
		GE_PROFILE_FUNCTION();

		while (p_Running)
		{
			{
				GE_PROFILE_SCOPE("App Time Compilation");
				float time = p_Window->GetTime();
				p_TS = time - p_LastFrameTime;
				p_LastFrameTime = time;
			}

			ExecuteMainThread();

			if (!p_Minimized)
			{
				GE_PROFILE_SCOPE("Updating App");

				// Reset Renderer & Bind Framebuffer
				Renderer::ResetStats();
				p_Framebuffer->Bind();
				
				// Updates Audio, Physics & Scripting
				Project::UpdateScene(p_TS);

				//	Updates Rendering
				for (Ref<Layer> layer : p_LayerStack->p_Layers)
				{
					layer->OnUpdate(p_TS);
				}
	
				p_Framebuffer->Unbind();
			}

			{ //	Updates Window
				GE_PROFILE_SCOPE("Updating AppWindow");
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

	/*
	* Used if Layers were inserted to the LayerStack directly
	*/
	void Application::AttachAllLayers()
	{
		for (auto& layer : p_LayerStack->p_Layers)
		{
			layer->OnAttach();
		}
	}

	void Application::PushLayer(Ref<Layer> layer)
	{
		GE_PROFILE_FUNCTION();

		if(p_LayerStack->InsertLayer(layer))
			layer->OnAttach();
	}

	void Application::PopLayer(Ref<Layer> layer)
	{
		GE_PROFILE_FUNCTION();

		if (p_LayerStack->RemoveLayer(layer))
			layer->OnDetach();
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
			p_MainThread = std::vector<std::function<void()>>();
		}

		for (auto& func : copy)
			func();

		copy.clear();
		copy = std::vector<std::function<void()>>();
	}

#pragma endregion

#pragma region Event Handling

	int Application::GetHovered(Framebuffer::Attachment format, const uint32_t& x, const uint32_t& y)
	{
		int pixelData = -1;
		if (x >= 0 && y >= 0 && x <= p_Framebuffer->GetWidth() && y <= p_Framebuffer->GetHeight())
		{
			pixelData = p_Framebuffer->ReadPixel(format, x, y);
		}
		return pixelData;
	}

	void Application::OnEvent(Event& e)
	{
		GE_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = p_LayerStack->end(); it != p_LayerStack->begin(); )
		{
			if (e.IsHandled())
				break;
			(*--it)->OnEvent(e);
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

		Renderer::ResizeViewport(0, 0, e.GetWidth(), e.GetHeight());
		Project::Resize(e.GetWidth(), e.GetHeight());
		p_Minimized = false;
		return true;
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
		const std::filesystem::path& path = Project::GetProjectPath() / std::filesystem::path(Project::GetName() + ".gproj");
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