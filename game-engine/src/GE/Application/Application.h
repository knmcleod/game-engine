#pragma once

#include "GE/Window/Window.h"
#include "GE/Window/Layers/LayerStack.h"
#include "GE/Window/Layers/imgui/ImGuiLayer.h"
#include "GE/Events/ApplicationEvent.h"

#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/VertexArray/VertexArray.h"

namespace GE
{

	class Application
	{
	public:
		Application();
		virtual ~Application();

		inline static Application& GetApplication() { return *s_Instance; };
		inline Window& GetWindow() { return *m_Window; };

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

	private:
		static Application* s_Instance;

		bool OnWindowClose(WindowCloseEvent& e);

		bool m_Running = true;

		std::unique_ptr<Window> m_Window;
		LayerStack m_LayerStack;

		ImGuiLayer* m_ImGuiLayer;

		//	Rendering Variables
		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		std::shared_ptr<VertexArray> m_VertexArray;
	};

	//Defined in Client
	Application* CreateApplication();
}

