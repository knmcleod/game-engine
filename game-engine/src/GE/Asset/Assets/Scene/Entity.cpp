#include "GE/GEpch.h"

#include "Entity.h"
#include "Scene.h"
#include "Components/Components.h"

#include "GE/Core/Input/Input.h"
#include "GE/Project/Project.h"

namespace GE
{
	Entity::Entity(uint32_t entityID) : p_EntityID(entityID)
	{

	}

	Entity::~Entity()
	{

	}

#pragma region OnComponentAdded

	template<typename T>
	void Entity::OnComponentAdded()
	{
		GE_CORE_ASSERT(false, "No OnCommponentAdded defined for Type");
	}

	template<>
	void Entity::OnComponentAdded<IDComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<TagComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<NameComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<ActiveComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<RelationshipComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<TransformComponent>()
	{

	}

#pragma region Audio

	template<>
	void Entity::OnComponentAdded<AudioSourceComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<AudioListenerComponent>()
	{

	}
#pragma endregion

#pragma region Rendering

	template<>
	void Entity::OnComponentAdded<RenderComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<CameraComponent>()
	{

	}

#pragma region 2D

	template<>
	void Entity::OnComponentAdded<SpriteRendererComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<CircleRendererComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<TextRendererComponent>()
	{

	}
#pragma endregion

#pragma region UI
	template<>
	void Entity::OnComponentAdded<GUIComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUICanvasComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUILayoutComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUIMaskComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUIImageComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUIButtonComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUIInputFieldComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUISliderComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUICheckboxComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<GUIScrollRectComponent>()
	{

	}
	template<>
	void Entity::OnComponentAdded<GUIScrollbarComponent>()
	{

	}
#pragma endregion

#pragma endregion

#pragma region Physics

	template<>
	void Entity::OnComponentAdded<Rigidbody2DComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<BoxCollider2DComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<CircleCollider2DComponent>()
	{
	}
#pragma endregion

#pragma region Scripting

	template<>
	void Entity::OnComponentAdded<NativeScriptComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<ScriptComponent>()
	{
	}
#pragma endregion

#pragma endregion

}