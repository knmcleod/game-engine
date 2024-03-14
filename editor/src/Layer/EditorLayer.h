#pragma once

#include <GE/GE.h>

#include "../UIPanels/SceneHierarchyPanel.h"
#include "../UIPanels/AssetPanel.h"

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
		void LoadScene(const std::filesystem::path& path);
		void SaveSceneFromFile();
		void SaveScene();
		void NewScene();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		// UI 

		void UI_Toolbar();

	private:
		EditorCamera m_EditorCamera;

		int m_StepFrameMultiplier = 1;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = glm::vec2();
		glm::vec2 m_ViewportBounds[2];

		Ref<Framebuffer> m_Framebuffer;
		Ref<Scene> m_EditorScene, m_ActiveScene;
		std::filesystem::path m_ScenePath;
		Scene::SceneState m_LastSceneState;

		Ref<SceneHierarchyPanel> m_ScenePanel;
		Ref<AssetPanel> m_AssetPanel;
		Ref<Texture2D> m_PlayButtonTexture, m_SimulateButtonTexture, m_PauseButtonTexture, m_StepButtonTexture, m_StopButtonTexture;

		Entity m_HoveredEntity;
		
	};
}
