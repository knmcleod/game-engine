#pragma once

#include <GE/GE.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "../UIPanels/SceneHierarchyPanel.h"

namespace GE
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& name = "Editor Layer");
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep timestep) override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;
		virtual void OnWindowResize(WindowResizeEvent& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		void LoadScene();
		void SaveSceneAs();
		void NewScene();

	private:
		OrthographicCameraController m_OrthoCameraController;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		Ref<Framebuffer> m_Framebuffer;
		Ref<Scene> m_ActiveScene;
		Ref<SceneHierarchyPanel> m_ScenePanel;

		Entity m_SquareEntity;
		Entity m_CameraEntityPrimary;
		Entity m_CameraEntitySecondary;
		bool m_CameraPrimary = true;

	};
}
