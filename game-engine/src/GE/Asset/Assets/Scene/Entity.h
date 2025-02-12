#pragma once

#include "GE/Core/Events/MouseEvent.h"
#include "GE/Core/Time/Timestep.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <entt/entt.hpp>
#pragma warning(pop)

namespace GE
{
	class Entity
	{
		friend class Scene;
	public:
		bool operator ==(const Entity& other) const { if (!other) return false; return p_EntityID == other.p_EntityID; }
		bool operator !=(const Entity& other) const { return !operator==(other); }
		operator bool() const { return ((entt::entity)p_EntityID != entt::null); }
		operator uint32_t() const { return p_EntityID; }
		operator entt::entity() const { return (entt::entity)p_EntityID; }

		template<typename T>
		void OnComponentAdded();

		Entity() = default;
		Entity(uint32_t entityID);
		virtual ~Entity();

	protected:
		inline void ClearEntityID() { p_EntityID = entt::null; }
	protected:
		uint32_t p_EntityID = entt::null;
	};

	class ScriptableEntity : public Entity
	{
		friend class Scene;
	public:
		bool operator ==(const Entity& other) const { if (!other) return false; return p_EntityID == (uint32_t)other; }
		bool operator !=(const Entity& other) const { return !operator==(other); }
		operator bool() const { return ((entt::entity)p_EntityID != entt::null); }
		operator uint32_t() const { return p_EntityID; }
		operator entt::entity() const { return (entt::entity)p_EntityID; }

		ScriptableEntity(uint32_t id) : Entity(id)
		{

		}

	protected:
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;
		virtual void OnUpdate(Timestep ts) = 0;
		virtual void OnEvent(Event& e) = 0;
	};

}