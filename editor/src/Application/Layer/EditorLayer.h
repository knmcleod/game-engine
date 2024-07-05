#pragma once

#include "../../Camera/EditorCamera.h"
#include "../../UIPanels/SceneHierarchyPanel.h"
#include "../../UIPanels/AssetPanel.h"

#include <GE/Core/Application/Layers/Layer.h>
#include <GE/Core/Events/KeyEvent.h>
#include <GE/Core/Events/MouseEvent.h>

#include <GE/Rendering/Framebuffers/Framebuffer.h>

namespace GE
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& name = "Editor Layer");
		virtual ~EditorLayer() override {}

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;
		virtual void OnWindowResize(WindowResizeEvent& e) override;

		const Scene::State& GetSceneState() { return m_RuntimeScene->GetState(); }
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMousePressed(MouseButtonPressedEvent& e);

		void OnDuplicateEntity();

		void StartScene(const Scene::State& state = Scene::State::Stop);
		void UpdateScene(Timestep ts);
		// Stop active processes & reverts to stopped/saved scene asset
		void StopScene();

		void LoadScene(UUID handle);

		// UI 
		void UI_Toolbar();

	private:
		EditorCamera* m_EditorCamera = nullptr;
		bool m_UseEditorCamera = true;
		int m_StepFrameMultiplier = 1;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		glm::vec2 m_ViewportBounds[2];

		Ref<Framebuffer> m_Framebuffer;
		Ref<SceneHierarchyPanel> m_ScenePanel;
		Ref<AssetPanel> m_AssetPanel;

		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;

		UUID m_PlayButtonHandle, m_SimulateButtonHandle, m_PauseButtonHandle, m_StepButtonHandle, m_StopButtonHandle;
		
		Entity m_HoveredEntity;
		glm::vec4 m_HoveredColor = glm::vec4(1.0f);;
	};
}
