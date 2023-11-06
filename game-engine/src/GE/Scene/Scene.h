#pragma once
#include <entt/entt.hpp>

#include "Components/Components.h"
namespace GE
{
	class Scene
	{
		friend class Entity;
	private:
		entt::registry m_Registry;
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "");

		void OnUpdate(Timestep timestep);
	};
}