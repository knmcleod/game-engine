#pragma once

#include "Components/Components.h"

#include "GE/Core/Events/MouseEvent.h"
#include "GE/Core/Time/Timestep.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <entt/entt.hpp>
#pragma warning(pop)

namespace GE
{
	// Forward declaration to avoid including Scene.h in this header
	class Scene;

	class Entity
	{
		friend class Scene;
	public:
		operator entt::entity() const { return (entt::entity)p_EntityID; }
		operator bool() const { return ((entt::entity)p_EntityID != entt::null && p_Scene); }
		operator uint32_t() const { return p_EntityID; }

		bool operator ==(const Entity& other) const { if (!other) return false; return p_EntityID == other.p_EntityID; }
		bool operator !=(const Entity& other) const { return !operator==(other); }

		template<typename T>
		bool HasComponent() const
		{
			if (!p_Scene)
			{
				GE_CORE_ERROR("Failed to check if Entity HasComponent<T>(). No Scene assigned.");
				return false;
			}
			return p_Scene->HasComponent<T>(*this);
		}

		template<typename T>
		void RemoveComponent()
		{
			if (HasComponent<T>())
				p_Scene->RemoveComponent<T>(*this);
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			GE_CORE_ASSERT(p_Scene, "Entity doesn't have Scene.");
			return p_Scene->AddOrReplaceComponent<T, Args...>(*this, args...);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			GE_CORE_ASSERT(p_Scene, "Entity doesn't have Scene.");
			return p_Scene->AddComponent<T, Args...>(*this, args...);
		}

		template<typename T>
		T& GetComponent() const
		{
			GE_CORE_ASSERT(p_Scene, "Entity doesn't have Scene.");
			return p_Scene->GetComponent<T>(*this);
		}

		template<typename T, typename... Args>
		T& GetOrAddComponent(Args&&... args)
		{
			if (!HasComponent<T>())
				return AddComponent<T>();
			return GetComponent<T>();
		}

		template<typename T>
		void OnComponentAdded();

		Entity() = default;
		Entity(uint32_t entityID, Scene* scene = nullptr);
		virtual ~Entity();

		const uint32_t& GetEntityID() const { return p_EntityID; }
		const Scene* GetScene() { return p_Scene; }

		virtual void OnEvent(Event& e);

	protected:
		inline void ClearEntityID() { p_EntityID = entt::null; }
		void SetID(uint32_t id, Ref<Scene> scene = nullptr)
		{
			p_EntityID = id;
			if (scene != nullptr && p_Scene != scene.get())
				p_Scene = scene.get();
		}

		bool OnMousePressed(MouseButtonPressedEvent& e);
		bool OnMouseReleased(MouseButtonReleasedEvent& e);

	protected:
		uint32_t p_EntityID = entt::null;
		Scene* p_Scene = nullptr;
	};

	class ScriptableEntity : public Entity
	{
		friend class Scene;
	public:
		operator entt::entity() const { return (entt::entity)p_EntityID; }
		operator bool() const { return (entt::entity)p_EntityID != entt::null; }
		operator uint32_t() const { return (uint32_t)p_EntityID; }

		bool operator ==(const Entity& other) const { return this->operator==(other); }
		bool operator !=(const Entity& other) const { return !operator==(other); }

		ScriptableEntity(uint32_t id, Scene* scene) : Entity(id, scene)
		{

		}

	protected:
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;
		virtual void OnUpdate(Timestep ts) = 0;
	};

}