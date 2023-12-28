#pragma once

#include "Entity.h"

namespace GE
{
	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetEntityComponent() { return m_Entity.GetComponent<T>(); }

	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(Timestep timestep) {}
	private:
		Entity m_Entity;
		friend class Scene;
	};

}