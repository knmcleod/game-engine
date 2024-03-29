#pragma once

#include "GE/Core/UUID/UUID.h"

#include "GE/Rendering/Text/Font.h"

#include "GE/Scene/Camera/SceneCamera.h"
#include "GE/Scene/Scene.h"

#include <glm/glm.hpp>

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
		glm::mat4 s_IdentityMat4 = glm::mat4{ 1.0f };
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) {}
	
		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::rotate(s_IdentityMat4, Rotation.x, { 1, 0, 0 })
				* glm::rotate(s_IdentityMat4, Rotation.y, { 0, 1, 0 })
				* glm::rotate(s_IdentityMat4, Rotation.z, { 0, 0, 1 });

			return glm::translate(s_IdentityMat4, Translation)
				* rotation * glm::scale(s_IdentityMat4, Scale);
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color = glm::vec4(1.0f);
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
	};

	struct CircleRendererComponent
	{
		glm::vec4 Color = glm::vec4(1.0f);
		float Radius = 0.5f; // Game Engine is 1x1, thus radius = 1/2;
		float Thickness = 0.5f; // 1: Full, 0: Empty
		float Fade = 0.0; // Blurs Circle 1: Full Fade 0: No Fade

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct TextRendererComponent
	{
		glm::vec4 TextColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
		glm::vec4 BGColor = glm::vec4(0.0);
		float KerningOffset = 0.0f; // Adjusts the spacing between characters proportionally
		float LineHeightOffset = 0.0f;
		Ref<Font> Font = Font::GetDefault();

		std::string Text = "Text";

		TextRendererComponent() = default;
		TextRendererComponent(const TextRendererComponent&) = default;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = false;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const SceneCamera camera) : Camera(camera) {}
	};

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
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

#pragma endregion

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
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f; // For bounciness
		float RestitutionThreshold = 0.5f;
		bool Show = true;

		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f; // For bounciness
		float RestitutionThreshold = 0.5f;

		bool Show = true;
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

#pragma endregion
	
}