#include "GE/GEpch.h"

#include "Entity.h"

namespace GE
{
	Entity::Entity(entt::entity entityID, Scene* scene) : m_EntityID(entityID), m_Scene(scene)
	{

	}
}