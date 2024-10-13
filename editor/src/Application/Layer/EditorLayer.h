#pragma once

#include "../../AssetManager/EditorAssetManager.h"
#include "../../Camera/EditorCamera.h"
#include "../../ImGUIPanels/SceneHierarchyPanel.h"
#include "../../ImGUIPanels/AssetPanel.h"

#include <GE/Core/Application/Layer/Layer.h>

namespace GE
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		EditorLayer(uint32_t id, bool isBase = false);
		~EditorLayer() override {}

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& e) override;
	private:
		void OnRender(Camera* camera) override;
		bool OnKeyPressed(KeyPressedEvent& e) override;
		bool OnMousePressed(MouseButtonPressedEvent& e) override;

		void OnDuplicateEntity(Entity e);

		UUID PopSelectedID();

		void StartScene(const Scene::State& state = Scene::State::Run);
		// Stop active processes & reverts to stopped/saved scene asset
		void StopScene();

		/*
		* If handle is provided, it'll be set
		* otherwise, Project::RuntimeScene will be reset & Scene & Asset Panel will be created
		*/
		void LoadScene(UUID handle = 0);

		// ImGUI 
		uint32_t ImGUI_WidgetID() const;

		void ImGUI_OnAttach();
		void ImGUI_OnDetach();
		void ImGUI_OnEvent(Event& e) const;

		void ImGUI_SceneToolbar();
		void ImGUI_Begin();
		void ImGUI_End();
		void ImGUI_Render();

		inline void ImGUI_BlockEvents(bool blockEvents) { m_ImGUIBlockEvents = blockEvents; }

	private:
		Camera* m_EditorCamera = nullptr;
		bool m_UseEditorCamera = true;
		bool m_ShowColliders = true;
		glm::vec4 m_ColliderColor = glm::vec4(0, 1, 0, 1);
		float m_ColliderModifier = 2.0f;
		float m_CircleColliderThickness = 0.5f;
		float m_CircleColliderFade = 0.4f;
		int m_StepFrameMultiplier = 1;

		bool m_ImGUIBlockEvents = true;
		bool m_ImGUIViewportFocused = false;
		bool m_ImGUIViewportHovered = false;
		glm::vec2 m_ImGUIViewportBounds[2];
		glm::vec2 m_ImGUIViewport;
		glm::vec2 m_ImGUIMousePosition;

		Ref<SceneHierarchyPanel> m_ScenePanel = nullptr;
		Ref<AssetPanel> m_AssetPanel = nullptr;

		UUID m_PlayButtonHandle, m_PauseButtonHandle, m_StepButtonHandle, m_StopButtonHandle = 0;
		UUID m_FontHandle = 0;

		Entity m_HoveredEntity = Entity();
		glm::vec4 m_HoveredColor = glm::vec4(1.0f);
	};
}
