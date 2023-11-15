#pragma once
#include <entt/entt.hpp>
#include "GE/Core/Time/Time.h"
#include "GE/Rendering/Camera/EditorCamera.h"

namespace GE
{
	class Scene
	{
		friend class SceneSerializer;
		friend class Entity;
		friend class SceneHierarchyPanel;
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		template<typename T>
		void OnComponentAdded(Entity entity);
	public:
		Scene() = default;
		~Scene() = default;

		Entity GetPrimaryCameraEntity();

		Entity CreateEntity(const std::string& name = "");
		void DestroyEntity(Entity entity);

		void ResizeViewport(uint32_t width, uint32_t height);

		void OnUpdateEditor(Timestep timestep, EditorCamera& camera);
		void OnUpdateRuntime(Timestep timestep);
	};
}