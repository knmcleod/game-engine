#pragma once

#include <imgui/imgui.h>

#include "GE/Core/Core.h"
#include "GE/Scene/Scene.h"
#include "GE/Scene/Entity/Entity.h"
#include "GE/Scene/Components/Components.h"
namespace GE
{
	class SceneHierarchyPanel
	{
	private:
		Ref<Scene> m_Scene;
		Entity m_SelectedEntity;

		void DrawEntity(Entity entity);
	public:
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetScene(const Ref<Scene>& scene);

		void OnImGuiRender();
	};
}