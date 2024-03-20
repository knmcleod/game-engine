#pragma once

#include "GE/Core/Application/Window/Window.h"
#include "GE/Core/Application/Layers/LayerStack.h"

#include "GE/Core/Events/ApplicationEvent.h"

#include "GE/ImGui/ImGuiLayer.h"

namespace GE
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			GE_CORE_ASSERT(index < Count, "Cannot assign Argument to index in-use.");
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Game Engine";
		std::string WorkingDirectory;

		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification specification);
		virtual ~Application();

		inline static Application& GetApplication() { return *s_Instance; };
		inline ApplicationSpecification& GetSpecification() { return m_Specification; };
		inline Window& GetWindow() { return *m_Window; };
		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		// Runs Window - Updates Layers and Timestep
		void Run();
		void Close();
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		void SubmitToMainThread(const std::function<void()>& function);

		void OnEvent(Event& e);
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void ExecuteMainThread();
	private:
		ApplicationSpecification m_Specification;

		std::vector<std::function<void()>> m_MainThread;
		std::mutex m_MainThreadMutex;

		Scope<Window> m_Window;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		bool m_Running = true;
		bool m_Minimized = false;

		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

	//Defined in Client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}

