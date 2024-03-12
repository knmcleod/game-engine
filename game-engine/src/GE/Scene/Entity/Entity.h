#pragma once
#include "GE/Core/UUID/UUID.h"

#include "GE/Scene/Scene.h"
#include "GE/Scene/Components/Components.h"

#include <entt/entt.hpp>
namespace GE
{
	class Entity
	{
		friend class Scene;
	private:
		entt::entity m_EntityID{ entt::null };
		Scene* m_Scene = nullptr;

		Scene* GetScene() { return m_Scene; }
	public:
		operator entt::entity() const { return m_EntityID; }
		operator bool() const { return m_EntityID != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityID; }

		bool operator ==(const Entity& other) const { return m_EntityID == other.m_EntityID && m_Scene == other.m_Scene; }
		bool operator !=(const Entity& other) const { return !operator==(other); }

		Entity() = default;
		Entity(entt::entity entityID, Scene* scene);

		entt::entity GetEntityID() { return m_EntityID; }

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityID, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this);
			return component;
		}

		template<typename T, typename... Args>
		T& GetOrAddComponent(Args&&... args)
		{
			if (!HasComponent<T>())
				return AddComponent<T>();
			return GetComponent<T>();
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			GE_CORE_ASSERT(!this->HasComponent<T>(), "Component already exists on Entity!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityID, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this);
			return component;
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

		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }
	
		
	};
}