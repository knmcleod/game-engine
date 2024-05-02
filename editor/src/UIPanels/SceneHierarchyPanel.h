#pragma once

#include "GE/Core/Core.h"

#include "GE/Asset/Assets/Scene/Scene.h"

namespace GE
{
	class SceneHierarchyPanel
	{
	private:
		Ref<Scene> m_Scene;
		Entity m_SelectedEntity;

		void DrawEntity(Entity entity);
		void DrawComponents(Entity entity);
	public:
		SceneHierarchyPanel(Ref<Scene> scene);

		void SetScene(Ref<Scene> scene);

		void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }
		Entity GetSelectedEntity() const { return m_SelectedEntity; }

		void OnImGuiRender();
	};
}