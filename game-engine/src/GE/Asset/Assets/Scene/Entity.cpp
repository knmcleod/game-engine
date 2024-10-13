#include "GE/GEpch.h"

#include "Entity.h"
#include "Scene.h"

#include "GE/Core/Input/Input.h"
#include "GE/Project/Project.h"

namespace GE
{
	static std::string GetMouseButtonStringFromKey(int key)
	{
		switch (key)
		{
		case Input::MOUSE_BUTTON_1:
			return "Mouse1";
			break;

		case Input::MOUSE_BUTTON_2:
			return "Mouse2";
			break;

		case Input::MOUSE_BUTTON_3:
			return "Mouse3";
			break;

		case Input::MOUSE_BUTTON_4:
			return "Mouse4";
			break;

		case Input::MOUSE_BUTTON_5:
			return "Mouse5";
			break;

		case Input::MOUSE_BUTTON_6:
			return "Mouse6";
			break;

		case Input::MOUSE_BUTTON_7:
			return "Mouse7";
			break;

		case Input::MOUSE_BUTTON_8:
			return "Mouse8";
			break;

		default:
			GE_CORE_ERROR("Unrecognized mouse key");
			break;
		}
		return "None";
	}

	Entity::Entity(uint32_t entityID, Scene* scene) : p_EntityID(entityID), p_Scene(scene)
	{
		if (p_Scene == nullptr && Project::GetRuntimeScene())
			p_Scene = Project::GetRuntimeScene().get();
	}

	Entity::~Entity()
	{
		p_Scene = nullptr;
		ClearEntityID();
	}

	void Entity::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseButtonPressedEvent>(GE_BIND_EVENT_FN(Entity::OnMousePressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(GE_BIND_EVENT_FN(Entity::OnMouseReleased));

	}

	bool Entity::OnMousePressed(MouseButtonPressedEvent& e)
	{
		std::string name = std::string("No Name");
		if (this->HasComponent<NameComponent>())
			name = this->GetComponent<NameComponent>().Name;
		std::string buttonString = GetMouseButtonStringFromKey(e.GetMouseButton());
		GE_CORE_TRACE("Entity: {0} \n\tMouseButtonPressed : {1}", name.c_str(), buttonString.c_str());

		return false;
	}

	bool Entity::OnMouseReleased(MouseButtonReleasedEvent& e)
	{
		std::string name = std::string("No Name");
		if (this->HasComponent<NameComponent>())
			name = this->GetComponent<NameComponent>().Name;
		std::string buttonString = GetMouseButtonStringFromKey(e.GetMouseButton());
		GE_CORE_TRACE("Entity: {0} \n\tMouseButtonReleased : {1}", name.c_str(), buttonString.c_str());
		return false;
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
	void Entity::OnComponentAdded<TransformComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<AudioSourceComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<AudioListenerComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<RenderComponent>()
	{

	}

	template<>
	void Entity::OnComponentAdded<CameraComponent>()
	{
		GetOrAddComponent<RenderComponent>(false);
	}

	template<>
	void Entity::OnComponentAdded<SpriteRendererComponent>()
	{
		GetOrAddComponent<RenderComponent>(false);
	}

	template<>
	void Entity::OnComponentAdded<CircleRendererComponent>()
	{
		GetOrAddComponent<RenderComponent>(false);
	}

	template<>
	void Entity::OnComponentAdded<TextRendererComponent>()
	{
		GetOrAddComponent<RenderComponent>(false);
	}

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

	template<>
	void Entity::OnComponentAdded<NativeScriptComponent>()
	{
	}

	template<>
	void Entity::OnComponentAdded<ScriptComponent>()
	{
	}
#pragma endregion
}