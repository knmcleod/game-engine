#pragma once

#include "GE/Core/Application/Window/Window.h"
#include "GE/Core/Application/Layers/LayerStack.h"

#include "GE/Core/Events/ApplicationEvent.h"

#include "GE/Core/Debug/ImGui/ImGuiLayer.h"

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

		inline static void SetFullscreen(bool fs) { return s_Instance->SetFullscreen(fs); }
		inline static const Application& GetApp() { return *s_Instance; };
		inline static void CloseApp() { s_Instance->Close(); }

		inline static void SaveAppProject() { s_Instance->SaveProject(); }
		inline static void SaveAppProjectFileDialog() { s_Instance->SaveProjectFileDialog(); }
		inline static void LoadAppProject() { s_Instance->LoadProject(); }
		inline static void LoadAppProjectFileDialog() { s_Instance->LoadProjectFileDialog(); }

		inline static void SubmitToMainAppThread(const std::function<void()>& func) { s_Instance->SubmitToMainThread(func); }

		inline static void BlockAppEvents(bool blockEvents) { s_Instance->p_ImGuiLayer->BlockEvents(blockEvents); }
		inline static const uint32_t GetActiveWidgetID() { return s_Instance->p_ImGuiLayer->GetActiveWidgetID(); }

		/*
		* Creates Window, debug Layer and Project with RuntimeAssetManager
		*/
		Application(const Config& config);
		virtual ~Application();

		inline const Config& GetConfig() const { return p_Config; };

		inline const Window& GetWindow() const { return *p_Window; };
		inline const ImGuiLayer& GetImGuiLayer() const { return *p_ImGuiLayer; }

		const uint32_t& GetWidth() const;
		const uint32_t& GetHeight() const;

		// Runs Window - Updates Layers and Timestep
		void Run();
		void Close();

	protected:
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		void OnEvent(Event& e);
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void ExecuteMainThread();
		void SubmitToMainThread(const std::function<void()>& func);

		/*
		* Assumes CommandlineArgs have been assigned
		*	Project file path found at index 1
		* If Args[1] found, LoadProject(Args[1])
		* , else LoadProjectFileDialog()
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

		// TODO: Thread wrapper class
		std::vector<std::function<void()>> p_MainThread;
		std::mutex p_MainThreadMutex;

		Scope<Window> p_Window;
		LayerStack p_LayerStack;
		ImGuiLayer* p_ImGuiLayer = nullptr;

		bool p_Running = true;
		bool p_Minimized = false;

		Timestep p_TS;
		float p_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

	//Defined in Client
	Application* CreateApplication(Application::CommandLineArgs args);
}

