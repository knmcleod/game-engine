#pragma once
#include <entt/entt.hpp>

#include "../Scene.h"

namespace GE
{
	class Entity
	{
	private:
		entt::entity m_EntityID{ entt::null };
		Scene* m_Scene = nullptr;
	public:
		Entity() = default;
		Entity(entt::entity entityID, Scene* scene);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			GE_CORE_ASSERT(!HasComponent<T>(), "Component already exists on Entity!");
			return m_Scene->m_Registry.emplace<T>(m_EntityID, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			GE_CORE_ASSERT(HasComponent<T>(), "Component doesn't exist on Entity!");
			return m_Scene->m_Registry.get<T>(m_EntityID);
		}

		template<typename T>
		void RemoveComponent()
		{
			GE_CORE_ASSERT(HasComponent<T>(), "Component doesn't exist on Entity!");
			m_Scene->m_Registry.remove<T>(m_EntityID);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.all_of<T>(m_EntityID);
		}

		operator bool() const { return m_EntityID != entt::null; }
	};
}