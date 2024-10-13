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
		Entity GetSelectedEntity() const;

		void OnImGuiRender();

	private:
		void SetSelected(UUID selectedEntity = 0);

		void DrawEntity(Entity entity);
		void DrawComponents(Entity entity);
	private:
		UUID m_SelectedEntityID = 0;
		glm::vec4 m_SelectedColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		std::string m_NewTagStr = std::string();
		std::string m_NewLayerStr = std::string();
	};
}