#pragma once

#include "GE/Asset/Assets/Scene/Entity.h"

#include "GE/Core/Application/Layer/LayerStack.h"
#include "GE/Core/Application/Window/Window.h"

#include "GE/Core/Events/ApplicationEvent.h"
#include "GE/Core/Input/Input.h"

#include "GE/Rendering/Framebuffers/Framebuffer.h"

namespace GE
{
	class Application
	{
	public:
		struct CommandLineArgs
		{
			int Count = 0;
			char** Args = nullptr;

			const char* operator[](int index) const
			{
				GE_CORE_ASSERT(index < Count, "Cannot get Argument index.");
				return Args[index];
			}
		};

		struct Config
		{
			std::string Name = "Game Engine";
			std::string WorkingDirectory;

			// Args[1]: project path
			CommandLineArgs Args;
		};

		inline static void CloseApp() { s_Instance->Close(); }

		inline static void SaveAppProject() { s_Instance->SaveProject(); }
		inline static void SaveAppProjectFileDialog() { s_Instance->SaveProjectFileDialog(); }
		inline static void LoadAppProject() { s_Instance->LoadProject(); }
		inline static void LoadAppProjectFileDialog() { s_Instance->LoadProjectFileDialog(); }

		inline static void SubmitToMainAppThread(const std::function<void()>& func) { s_Instance->SubmitToMainThread(func); }

		inline static Ref<LayerStack> GetLayerStack() { return s_Instance->p_LayerStack; }
		template<typename T>
		inline static Ref<T> GetLayerStack() { return static_ref_cast<T, LayerStack>(s_Instance->p_LayerStack); }
		inline static const std::vector<Ref<Layer>>& GetLayers() { return s_Instance->p_LayerStack->p_Layers; }

		inline static void* GetNativeWindow() { return s_Instance->p_Window->GetNativeWindow(); }
		inline static uint64_t GetWindowWidth() { return s_Instance->p_Window->GetWidth(); }
		inline static uint64_t GetWindowHeight() { return s_Instance->p_Window->GetHeight(); }
		inline static void SetFullscreen(bool fs) { s_Instance->p_Window->SetFullscreen(fs); }

		inline static Ref<Framebuffer> GetFramebuffer() { return s_Instance->p_Framebuffer; }
		inline static void UnbindFramebuffer() { s_Instance->p_Framebuffer->Unbind(); }
		inline static void BindFramebuffer() { s_Instance->p_Framebuffer->Bind(); }
		/*
		* Returns Entity at (x,y) relative to Framebuffer::Bounds. 
		* Use modifier to change Bounds relativity(+/-).
		* @param x : horizontal mouse position, default Input::MouseX
		* @param x : vertical mouse position, default Input::MouseY
		* @param modifier : addition(+1) or subtraction(-1), default -1
		*/
		static Entity GetHoveredEntity(float x = Input::GetMouseX(), float y = Input::GetMouseY(), int modifier = -1);
		/*
		* Returns mouse position that is relative to Framebuffer::Config::Bounds{ {minX, minY}, { maxX, maxY} }
		* @param x : horizontal mouse position
		* @param y : vertical mouse position
		* @param modifier : add(+1) or subtract(-1), default -1
		*/
		static const glm::vec2 GetRelativeMouse(float x, float y, int modifier = -1);
	public:
		/*
		* Creates Window, debug Layer and Project with RuntimeAssetManager
		*/
		Application(const Config& config);
		virtual ~Application();

		inline const Config& GetConfig() const { return p_Config; };

		// Updates Timestep and Layers 
		void Run();
		void Close();

	protected:
		/*
		* Attaches all Layers already in LayerStack
		*/
		void AttachAllLayers();
		void PushLayer(Ref<Layer> layer);
		void PopLayer(Ref<Layer> layer);

		void OnEvent(Event& e);
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void ExecuteMainThread();
		void SubmitToMainThread(const std::function<void()>& func);

		/*
		* Assumes CommandlineArgs have been assigned
		*	Project file path found at index 1
		* If Args[1] found, LoadProject(Args[1])
		* else LoadProjectFileDialog()
		*/
		bool LoadProject() const;
		bool LoadProjectFileDialog() const;
		bool LoadProject(const std::filesystem::path& path) const;

		bool SaveProject() const;
		bool SaveProjectFileDialog() const;
		/*
		* Full Project path is assumed.
		* Project Extension is handled.
		* Example path: projects/projectName
		*/
		bool SaveProject(const std::filesystem::path& path) const;

		int GetHovered(Framebuffer::Attachment format, const uint32_t& x, const uint32_t& y);
	protected:
		Config p_Config;

		Scope<Window> p_Window = nullptr;
		Ref<LayerStack> p_LayerStack = nullptr;

		bool p_Running = true;
		bool p_Minimized = false;

		Timestep p_TS;
		float p_LastFrameTime = 0.0f;

		Ref<Framebuffer> p_Framebuffer = nullptr;

		// TODO: Thread wrapper class
		std::vector<std::function<void()>> p_MainThread;
		std::mutex p_MainThreadMutex;

	private:
		static Application* s_Instance;
	};

	//Defined in Client
	Application* CreateApplication(Application::CommandLineArgs args);
}

