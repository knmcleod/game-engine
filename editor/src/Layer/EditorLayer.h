#pragma once

#include "../UIPanels/SceneHierarchyPanel.h"
#include "../UIPanels/AssetPanel.h"

#include <GE/Core/Application/Layers/Layer.h>
#include <GE/Core/Events/KeyEvent.h>

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
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;
		virtual void OnWindowResize(WindowResizeEvent& e) override;

		const Scene::State& GetSceneState() { return m_ActiveScene->GetState(); }
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMousePressed(MouseButtonPressedEvent& e);

		void OnDuplicateEntity();

		void StartScene(const Scene::State& state = Scene::State::Stop);
		void UpdateScene(Timestep ts, Camera* camera = nullptr);
		void StopScene();

		void LoadSceneFromFile();
		void LoadScene(UUID handle);

		bool LoadProjectFromFile();
		bool LoadProject(const std::filesystem::path& path);
		void SaveProjectFromFile();
		/*
		* Full Project path is assumed. See demo
		* Project Extension is handled.
		* Example path: projects/projectName
		*/
		void SaveProject(const std::filesystem::path& path);
		void SaveProject();

		// UI 
		void UI_Toolbar();

	private:
		EditorCamera m_EditorCamera;
		bool m_UseEditorCamera = true;
		int m_StepFrameMultiplier = 1;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = glm::vec2();
		glm::vec2 m_ViewportBounds[2];

		Ref<Framebuffer> m_Framebuffer;
		Ref<SceneHierarchyPanel> m_ScenePanel;
		Ref<AssetPanel> m_AssetPanel;

		Ref<Scene> m_ActiveScene;

		UUID m_PlayButtonHandle, m_SimulateButtonHandle, m_PauseButtonHandle, m_StepButtonHandle, m_StopButtonHandle;
		
		Entity m_HoveredEntity;
	};
}
