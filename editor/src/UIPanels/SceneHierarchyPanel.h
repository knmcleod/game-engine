#pragma once

#include "GE/Core/Core.h"

#include "GE/Asset/Assets/Scene/Scene.h"

namespace GE
{
	class SceneHierarchyPanel
	{
		friend class EditorLayer;
	public:
		SceneHierarchyPanel(Scene* scene);

		inline const Entity& GetSelectedEntity() const { return m_SelectedEntity; }
		inline const glm::vec4& GetSelectedColor() const { return m_SelectedColor; }

		void OnImGuiRender();
	private:
		void SetScene(Scene* scene);

		void DrawEntity(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Scene* m_Scene;
		Entity m_SelectedEntity;
		glm::vec4 m_SelectedColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	};
}