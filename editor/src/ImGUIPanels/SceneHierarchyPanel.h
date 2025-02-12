#pragma once

#include "GE/Asset/Assets/Scene/Scene.h"

#include <glm/glm.hpp>

namespace GE
{
	class SceneHierarchyPanel
	{
		friend class EditorLayer;
	public:
		template<typename UIFunc>
		static void DrawPopup(const std::string& name, Entity entity, UIFunc func);

		static void DrawInputText(const std::string& label, std::string& text);
		
		SceneHierarchyPanel() = default;
		~SceneHierarchyPanel();

		inline const glm::vec4& GetSelectedColor() const { return m_SelectedColor; }
		inline void ClearSelected() { m_SelectedEntityID = 0; }
	
		void OnImGuiRender(Ref<Scene> scene);
	private:
		void SetSelected(UUID selectedEntity = 0);

		/*
		* Using ImGui, draws Entity with children in hierarchy panel
		* @param scene : active scene
		* @param child : entity to draw
		*/
		void DrawEntity(Ref<Scene> scene, Entity entity);

		/*
		* Using ImGui, draws Entity Components in property panel
		*/
		void DrawComponents(Ref<Scene> scene, Entity entity);
	private:
		UUID m_SelectedEntityID = 0;
		glm::vec4 m_SelectedColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		std::string m_NewTagStr = std::string();
		std::string m_NewLayerStr = std::string();
	};
}