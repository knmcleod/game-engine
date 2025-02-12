#pragma once

#include "../../Camera/EditorCamera.h"
#include "../../ImGUIPanels/SceneHierarchyPanel.h"
#include "../../ImGUIPanels/AssetPanel.h"

#include <GE/Core/Application/Layer/Layer.h>

namespace GE
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer() = default;
		EditorLayer(uint64_t id);
		~EditorLayer() override {}

	private:
		/*
		* Renders entity & its children recursively.
		* 
		* @param scene : runtime scene
		* @param entity : entity to render
		* @param translationOffset : offset from entity to parent
		* @param rotationOffset : offset from entity to parent
		*/
		void RenderEntity(Ref<Scene> scene, Entity entity, glm::vec3& translationOffset, glm::vec3& rotationOffset) override;
		void OnAttach(Ref<Scene> scene) override;
		void OnDetach() override;

		void OnUpdate(Ref<Scene> scene, Timestep ts) override;
		void OnRender(Ref<Scene> scene, const Camera*& camera) override;

		void OnEvent(Event& e) override;
		bool OnKeyPressed(KeyPressedEvent& e) override;
		bool OnMousePressed(MousePressedEvent& e) override;

		void StartScene(const Scene::State& state = Scene::State::Run);
		// Stop active processes & reverts to stopped/saved scene asset
		void StopScene();

		/*
		* If handle is provided, it'll be set
		* Then, Project::RuntimeScene will be reset & Scene/Asset Panel will be created
		* 
		* @param handle : Optional Scene Asset Handle. Default 0
		*/
		void LoadScene(UUID handle = 0);

		// ImGUI 
		uint32_t ImGUI_WidgetID() const;

		void ImGUI_OnAttach();
		void ImGUI_OnDetach();
		void ImGUI_OnEvent(Event& e) const;

		void ImGUI_Begin();
		void ImGUI_End();
		void ImGUI_Update();
		void ImGUI_SceneToolbar(Ref<Scene> scene);

		inline void ImGUI_BlockEvents(bool blockEvents) { m_ImGUIBlockEvents = blockEvents; }

	private:
		EditorCamera* m_EditorCamera = nullptr;
		bool m_UseEditorCamera = true;

		bool m_ShowPivotPoints = true;
		glm::vec4 m_PivotColor = glm::vec4(0.75f, 1.0f, 1.0f, 1.0f);
		float m_PivotSize = 0.5f;
		float m_PivotThickness = 0.25f;
		float m_PivotFade = 0.2f;

		bool m_ShowColliders = true;
		glm::vec4 m_ColliderColor = glm::vec4(0, 1, 0, 1);
		float m_CircleColliderThickness = 1.0f;
		float m_CircleColliderFade = 0.95f;
		
		int m_StepFrameMultiplier = 1;

		Ref<SceneHierarchyPanel> m_ScenePanel = nullptr;
		Ref<AssetPanel> m_AssetPanel = nullptr;

		UUID m_PlayButtonHandle, m_PauseButtonHandle, m_StepButtonHandle, m_StopButtonHandle = 0;
		UUID m_FontHandle = 0;
		// TODO : Move to Project
		UUID m_EditorIconHandle = 0;

		Entity m_HoveredEntity;
		glm::vec4 m_HoveredColor = glm::vec4(1.0f);
		glm::vec4 m_SceneCameraViewportColor = glm::vec4(0.8f, 0.0, 0.5f, 1.0f);
		glm::vec4 m_GUIViewportColor = glm::vec4(0.0f, 0.0, 0.0f, 1.0f);

		bool m_ShowMouse = true;
	private: // ImGUI only
		bool m_ImGUIBlockEvents = true;
		bool m_ImGUIViewportFocused = false;
		bool m_ImGUIViewportHovered = false;
		glm::vec2 m_ImGUIViewportBounds[2];
		glm::vec2 m_ImGUIViewport;
		glm::vec2 m_ImGUIMousePosition;

		UUID m_AID = UUID();
		UUID m_ECID = UUID();
		UUID m_RSID = UUID();
		UUID m_SID = UUID();
		UUID m_PID = UUID();
		UUID m_CID = UUID();
	};
}
