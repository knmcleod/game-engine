#pragma once

#include "GE/Core/Core.h"
#include "GE/Scene/Scene.h"
#include "GE/Scene/Entity/Entity.h"

#include <glm/gtc/type_ptr.hpp>

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
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetScene(const Ref<Scene>& scene);

		void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }
		Entity GetSelectedEntity() const { return m_SelectedEntity; }

		void OnImGuiRender();
	};
}