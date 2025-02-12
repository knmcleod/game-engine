#pragma once

#include "Layer/LayerStack.h"
#include "Layer/GUILayer.h"
#include "Window/Window.h"

#include "GE/Asset/Assets/Scene/Entity.h"

#include "GE/Core/Events/ApplicationEvent.h"
#include "GE/Core/Input/Input.h"
#include "GE/Core/Memory/Thread.h"

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

		inline static void SubmitToMainAppThread(const std::function<void()>& func) { s_Instance->AddToMainThread(func); }

		static const Timestep& GetTimestep() { return s_Instance->p_TS; }
		inline static Ref<LayerStack> GetLayerStack() { return s_Instance->p_LayerStack; }
		template<typename T>
		inline static Ref<T> GetLayerStack() { return static_ref_cast<T, LayerStack>(s_Instance->p_LayerStack); }
		inline static const LayerStack::Layers& GetLayers() { return s_Instance->p_LayerStack->p_Layers; }

		inline static void* GetNativeWindow() { return s_Instance->p_Window->GetNativeWindow(); }
		inline static const uint64_t& GetWindowWidth() { return s_Instance->p_Window->GetWidth(); }
		inline static const uint64_t& GetWindowHeight() { return s_Instance->p_Window->GetHeight(); }
		inline static const std::pair<uint64_t, uint64_t> GetWindowPosition() { return s_Instance->p_Window->GetPosition(); }
		
		inline static bool IsFullscreen() { return s_Instance->p_Window->IsFullscreen(); }
		inline static void SetFullscreen(bool fs) { s_Instance->p_Window->SetFullscreen(fs); }
		inline static bool IsFramebufferHovered() { return s_Instance->FramebufferHovered(); }
		inline static bool IsMouseActive() { return s_Instance->p_Window->IsMouseActive(); }
		inline static bool IsMousePressed(const  Input::MouseCode& mouseCode) { return s_Instance->p_Window->IsMousePressed(mouseCode); }
		inline static bool IsKeyPressed(const Input::KeyCode& keyCode) { return s_Instance->p_Window->IsKeyPressed(keyCode); }
		
		inline static Input::CursorMode GetCursorMode() { return s_Instance->p_Window->GetCursorMode(); }
		inline static void SetCursorMode(const Input::CursorMode& mode) { s_Instance->p_Window->SetCursorMode(mode); }
		inline static void SetCursorShape(const Window::CursorShape& shape) { s_Instance->p_Window->SetCursorShape(shape); }
		inline static void SetCursorIcon(UUID textureHandle, uint32_t x, uint32_t y) { s_Instance->p_Window->SetCursorIcon(textureHandle, x, y); }

		inline static void SetIcon(UUID textureHandle) { s_Instance->p_Window->SetIcon(textureHandle); }

		inline static Ref<Framebuffer> GetFramebuffer() { return s_Instance->p_Framebuffer; }
		inline static void UnbindFramebuffer() { s_Instance->p_Framebuffer->Unbind(); }
		inline static void BindFramebuffer() { s_Instance->p_Framebuffer->Bind(); }
		/*
		* Returns Entity at (x,y) relative to Framebuffer::Bounds. 
		* Use modifier to change Bounds relativity(+/-).
		* @param mousePosition : 
		* @param modifier : addition(+1) or subtraction(-1), default -1
		*/
		static Entity GetHoveredEntity(const glm::vec2 mousePosition = GetWindowCursor(), int modifier = -1);
		
		/*
		* Returns windowPosition + cursor position
		* If in windowed mode, will include tabbar 
		*/
		static const glm::vec2 GetWindowCursor();
		/*
		* Returns mouse position that is relative to Framebuffer::Config::Bounds{ {minX, minY}, { maxX, maxY} }
		* @param mousePosition : window cursor position
		* @param modifier : add(+1) or subtract(-1), default -1
		*/
		static const glm::vec2 GetFramebufferCursor(const glm::vec2 mousePosition = GetWindowCursor(), int modifier = -1);

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
		int GetHovered(Framebuffer::Attachment format, const uint32_t& x, const uint32_t& y);
		virtual bool FramebufferHovered();
		void AddToMainThread(const std::function<void()>& func);
		void ExecuteMainThread();

		/*
		* Attaches all Layers already in LayerStack
		* Used if Layers were inserted to the LayerStack directly
		*/
		void AttachAllLayers();
		void PushLayer(Ref<Layer> layer);
		void PopLayer(Ref<Layer> layer);
		/*
		* Should be used for Rendering. 
		* Main Framebuffer will always be bound, unless unbound by Layer.
		* Called after Renderer reset & Framebuffer bind.
		*/
		void UpdateLayers();
		void EventLayers(Event& e);

		void OnEvent(Event& e);
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

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

	protected:
		Config p_Config;

		Scope<Window> p_Window = nullptr;
		Ref<LayerStack> p_LayerStack = nullptr;

		bool p_Running = true;
		bool p_Minimized = false;

		Timestep p_TS;
		float p_LastFrameTime = 0.0f;

		Ref<Framebuffer> p_Framebuffer = nullptr;

		Thread p_MainThread;
	private:
		static Application* s_Instance;
	};

	//Defined in Client
	Application* CreateApplication(Application::CommandLineArgs args);
}

