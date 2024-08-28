#pragma once

#include "GE/Core/UUID/UUID.h"

#include "GE/Rendering/Camera/SceneCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
	#include <glm/gtx/quaternion.hpp>

namespace GE
{	
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag = "";

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) {}
		
		/*
		* param translationOffset : added with Translation using glm::translate()
		* param scaleOffset : multiplied with Scale using glm::scale()
		*/
		const glm::mat4 GetTransform(glm::vec3& translationOffset = glm::vec3(0.0f), glm::vec3& scaleOffset = glm::vec3(1.0f)) const
		{
			glm::mat4 identityMat4 = glm::mat4(1.0f);
			glm::mat4 rotation = glm::rotate(identityMat4, Rotation.x, {1, 0, 0})
				* glm::rotate(identityMat4, Rotation.y, { 0, 1, 0 })
				* glm::rotate(identityMat4, Rotation.z, { 0, 0, 1 });

			return glm::translate(identityMat4, Translation + translationOffset)
				* rotation * glm::scale(identityMat4, Scale * scaleOffset);
		}
	};

	struct CameraComponent
	{
		SceneCamera ActiveCamera;
		bool Primary = false;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const SceneCamera camera) : ActiveCamera(camera) {}
	};

	struct AudioSourceComponent
	{
		UUID AssetHandle = 0;

		bool Loop = false;
		float Pitch = 1.0f;
		float Gain = 1.0f;

		AudioSourceComponent() = default;
		AudioSourceComponent(const AudioSourceComponent&) = default;
	};

	struct AudioListenerComponent
	{
		bool Test = false;

		AudioListenerComponent() = default;
		AudioListenerComponent(const AudioListenerComponent&) = default;
	};

	struct SpriteRendererComponent
	{
		UUID AssetHandle = 0;
		float TilingFactor = 1.0f;
		glm::vec4 Color = glm::vec4(1.0f);

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
	};

	struct CircleRendererComponent
	{
		UUID AssetHandle = 0;
		float TilingFactor = 1.0f;
		float Radius = 0.5f; // Game Engine is 1x1, thus radius = 1/2;
		float Thickness = 1.0f; // 1: Full, 0: Empty
		float Fade = 0.0; // Blurs Circle 1: Full Fade 0: No Fade
		glm::vec4 Color = glm::vec4(1.0f);

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct TextRendererComponent
	{
		UUID AssetHandle = 0;
		float KerningOffset = 0.0f; // Adjusts the spacing between characters proportionally
		float LineHeightOffset = 0.0f;
		std::string Text = "Text";
		glm::vec4 TextColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
		glm::vec4 BGColor = glm::vec4(0.0);

		TextRendererComponent() = default;
		TextRendererComponent(const TextRendererComponent&) = default;
	};

#pragma region Physics
	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		bool Show = true;
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f; // For bounciness
		float RestitutionThreshold = 0.5f;
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		bool Show = true;
		float Radius = 0.5f; // GE is 1x1
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f; // For bounciness
		float RestitutionThreshold = 0.5f;
		glm::vec2 Offset = { 0.0f, 0.0f };

		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

#pragma endregion

#pragma region Scripting

	class ScriptableEntity;
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	struct ScriptComponent
	{
		// TODO : Switch to UUID from Script : Asset, store ClassName in Script(?)
		std::string ClassName = std::string();

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

#pragma endregion

	namespace ComponentUtils
	{
		static const std::string GetStringFromRigidBody2DType(Rigidbody2DComponent::BodyType type)
		{
			switch (type)
			{
			case GE::Rigidbody2DComponent::BodyType::Static:
				return "Static";
				break;
			case GE::Rigidbody2DComponent::BodyType::Dynamic:
				return "Dynamic";
				break;
			case GE::Rigidbody2DComponent::BodyType::Kinematic:
				return "Kinematic";
				break;
			}
			GE_CORE_WARN("Cannot convert Rigidbody2D BodyType to string.");
			return "Static";
		}

		static Rigidbody2DComponent::BodyType GetRigidBody2DTypeFromString(const std::string& typeString)
		{
			if (typeString == "Static") return Rigidbody2DComponent::BodyType::Static;
			if (typeString == "Dynamic") return Rigidbody2DComponent::BodyType::Dynamic;
			if (typeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

			return Rigidbody2DComponent::BodyType::Static;
		}
	}
}