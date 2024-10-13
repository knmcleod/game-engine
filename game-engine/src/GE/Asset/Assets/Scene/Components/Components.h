#pragma once

#include "GE/Audio/AudioDevice.h"

#include "GE/Core/UUID/UUID.h"

#include "GE/Rendering/Camera/SceneCamera.h"

namespace GE
{	
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid) : ID(uuid)
		{
		}
	};

	struct TagComponent
	{
		// Corresponding TagStr can be found using Project::GetTagByKey(uint32_t)
		uint32_t ID = 0;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(uint32_t id) : ID(id)
		{
		}
	};

	struct NameComponent
	{
		std::string Name = std::string("New Entity");

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		NameComponent(const std::string& name) : Name(name) {}
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
		const glm::mat4 GetTransform(glm::vec3& translationOffset = glm::vec3(0.0f), glm::vec3& scaleOffset = glm::vec3(1.0f)) const;
	};

#pragma region Audio

	struct AudioSourceComponent
	{
		// Audio Buffer Asset
		UUID AssetHandle = 0;

		uint32_t ID = 0;
		bool Loop = false;
		float Pitch = 1.0f;
		float Gain = 1.0f; // Volume

		AudioSourceComponent() = default;
		AudioSourceComponent(const AudioSourceComponent&) = default;
		inline AudioSourceComponent(const uint32_t& sourceID, const UUID& handle) { ID = sourceID; AssetHandle = handle; }

		void Play(const glm::vec3& position = glm::vec3(), const glm::vec3& velocity = glm::vec3()) const;
		void Pause() const;
		void Stop() const;

		bool IsPlaying() const;
	};

	struct AudioListenerComponent
	{
		Ref<AudioDevice> Device = nullptr;
		AudioListenerComponent();
		AudioListenerComponent(const AudioListenerComponent&) = default;
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

#pragma region Renderer

	struct RenderComponent
	{
		// An Entity should only be rendered once
		bool Rendered = false;
		// All Layers Entity can be rendered
		std::vector<uint64_t> LayerIDs = std::vector<uint64_t>();
	
		RenderComponent() = default;
		RenderComponent(const RenderComponent& other)
		{
			Rendered = other.Rendered;
			if (!other.LayerIDs.empty())
				LayerIDs = other.LayerIDs;
		}
		RenderComponent(bool rendered, std::vector<uint64_t> layerIDs = std::vector<uint64_t>()) : Rendered(rendered)
		{
			if (!layerIDs.empty())
				LayerIDs = layerIDs;
		}

		~RenderComponent()
		{
			LayerIDs.clear();
			LayerIDs = std::vector<uint64_t>();
		}

		/*
		* Returns true if id exists in LayerIDs, i.e. Entity can be rendered on ID, see LayerStack::
		*/
		bool IDHandled(uint64_t id) const
		{
			auto it = std::find(LayerIDs.begin(), LayerIDs.end(), id);

			if (it != LayerIDs.end())
				return true;

			return false;
		}

		void AddID(uint64_t id)
		{
			if (!IDHandled(id))
				LayerIDs.insert(LayerIDs.begin() + LayerIDs.size(), id);
		}

		void RemoveID(uint64_t id)
		{
			bool remove = false;
			uint64_t index = 0;
			for (auto& layerID : LayerIDs)
			{
				if (layerID == id)
				{
					remove = true;
					break;
				}

				index++;
			}

			if(remove)
				LayerIDs.erase(LayerIDs.begin() + index);
		}
	};

	struct CameraComponent
	{
		SceneCamera ActiveCamera = SceneCamera();
		bool Primary = false;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const SceneCamera camera) : ActiveCamera(camera)
		{
		}

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
		CircleRendererComponent(const glm::vec4& color, const float& radius,
			const float& thickness, const float& fade)
			: Color(color), Radius(radius), Thickness(thickness), Fade(fade)
		{

		}
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

#pragma endregion

#pragma region Scripting

	class ScriptableEntity;
	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)() = nullptr;
		void (*DestroyScript)(NativeScriptComponent*) = nullptr;

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	struct ScriptComponent
	{
		// Script Asset Handle
		UUID AssetHandle = 0;

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