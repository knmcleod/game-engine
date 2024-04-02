#pragma once

#include "../UIPanels/SceneHierarchyPanel.h"
#include "../UIPanels/AssetPanel.h"

#include <GE/Core/Application/Layers/Layer.h>
#include <GE/Core/Events/KeyEvent.h>

#include <GE/Project/Project.h>
#include <GE/Rendering/Framebuffers/Framebuffer.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

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
		bool OnMousePressed(MouseButtonPressedEvent& e);

		void OnDuplicateEntity();

		void OnSceneRuntime();
		void OnSceneSimulate();
		void OnScenePause();
		void OnSceneStop();

		void LoadSceneFromFile();
		void LoadScene(UUID handle);
		bool SaveSceneFromFile();
		bool SaveScene(UUID handle);

		bool LoadProjectFromFile();
		void LoadProject(const std::filesystem::path& path);
		void SaveProjectFromFile();
		void SaveProject();
		Ref<Project> NewProject();

		// UI 
		void UI_Toolbar();

	private:
		EditorCamera m_EditorCamera;
		Entity m_HoveredEntity;

		bool m_UseEditorCamera = true;
		int m_StepFrameMultiplier = 1;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = glm::vec2();
		glm::vec2 m_ViewportBounds[2];

		Ref<Framebuffer> m_Framebuffer;
		Ref<SceneHierarchyPanel> m_ScenePanel;
		Ref<AssetPanel> m_AssetPanel;

		Ref<Scene> m_EditorScene, m_ActiveScene;
		SceneState m_LastSceneState;

		Ref<Texture2D> m_PlayButtonTexture, m_SimulateButtonTexture, m_PauseButtonTexture, m_StepButtonTexture, m_StopButtonTexture;
		
	};
}
