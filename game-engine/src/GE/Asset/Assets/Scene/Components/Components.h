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
		// Corresponding TagStr can be found using Project::GetStrByTag(TagID)
		uint32_t TagID = 0;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(uint32_t tag) : TagID(tag)
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

	struct ActiveComponent
	{
		// If Active, Entity will be visible & update
		// else, Entity won't be visible | active
		bool Active = true;

		// Only when Active
		// If Hidden, Entity will not be visible, but will update
		// else, Entity will be visible & update
		bool Hidden = false;
		ActiveComponent() = default;
		ActiveComponent(const ActiveComponent&) = default;
	};

	struct RelationshipComponent
	{
		friend class Scene;
		friend class RuntimeAssetManager;
	private:
		UUID Parent = 0;
		std::vector<UUID> Children = std::vector<UUID>();

	public:
		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent&) = default;
		RelationshipComponent(const UUID& parent, const std::vector<UUID>& children = std::vector<UUID>()) : Parent(parent)
		{
			Children = children;
		}

		~RelationshipComponent()
		{
			Children.clear();
			Children = std::vector<UUID>();
		}

		const UUID& GetParent() { return Parent; }
		const std::vector<UUID>& GetChildren() { return Children; }

		bool HasChild(const UUID& id);
		void AddChild(const UUID& id);
		void RemoveChild(const UUID& id);

		const UUID& GetFront() const;
		const UUID& GetBack() const;
		const UUID& GetNext(const UUID& id) const;
		const UUID& GetPrevious(const UUID& id) const;

	private:
		/*
		* Returns true if id is found
		*/
		bool GetChildIndex(const UUID& id, uint64_t& index) const;
	};

	enum Pivot
	{
		Center = 0,
		LowerLeft,
		TopLeft,
		TopRight,
		LowerRight,
		MiddleRight,
		TopMiddle,
		MiddleLeft,
		BottomMiddle,
	};

	struct TransformComponent
	{
	public:
		// Default (0, 0) Origin, will never include Pivot
		glm::vec3 Translation = glm::vec3(0.0f);
		// In degrees
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);
	private:
		Pivot PivotEnum = Pivot::Center;
		// Offset from Center(0,0)
		glm::vec3 PivotOffset = glm::vec3(0.0f);
	public:
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) {}
		
		const glm::vec3& GetPivotOffset() { return PivotOffset; }
		const Pivot& GetPivot() const { return PivotEnum; }

		/*
		* Returns Entitys Translation offset by Pivot
		*/
		const glm::vec3 GetOffsetTranslation() const;
		/*
		* Returns centered world transformation
		*
		* @param translationOffset : added with Translation using glm::translate()
		* @param rotationOffset : added with Rotation using glm::rotate()
		* @param scaleOffset : multiplied with Scale using glm::scale()
		* @param pivotOffset : offset from center
		*/
		const glm::mat4 GetTransform(const glm::vec3& translationOffset = glm::vec3(0.0f), const glm::vec3& rotationOffset = glm::vec3(0.0f), const glm::vec3& scaleOffset = glm::vec3(1.0f), const glm::vec3& pivotOffset = glm::vec3(0.0f)) const;

		/*
		* Returns min & max bounds of entity
		*/
		const std::pair<glm::vec2, glm::vec2> GetBounds() const;

		void SetPivot(const Pivot& pivot);
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
		glm::vec2 Size = { 1.0f, 1.0f };

		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		bool Show = true;
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f; // For bounciness
		float RestitutionThreshold = 0.5f;
		float Radius = 0.5f;
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

		void SetProjectionType(Camera::ProjectionType type)
		{
			ActiveCamera.SetProjectionType(type);
		}
		void SetFOV(float fov)
		{
			ActiveCamera.SetFOV(fov);
		}
		void SetNearClip(float nearclip)
		{
			ActiveCamera.SetNearClip(nearclip);
		}
		void SetFarClip(float farclip)
		{
			ActiveCamera.SetFarClip(farclip);
		}
		void SetPosition(const glm::vec3& position)
		{
			ActiveCamera.SetPosition(position);
		}
		
		void OnUpdate(Timestep ts)
		{
			ActiveCamera.OnUpdate(ts);
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
		float Radius = 0.5f; // GE is 1x1, r = 1/2
		float Thickness = 1.0f; // 1: Full, 0: Empty
		float Fade = 0.0; // Blurs Circle. Full Fade(1). No Fade(0).
		glm::vec4 Color = glm::vec4(1.0f);

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
		CircleRendererComponent(const float& thickness, const float& fade, const glm::vec4& color)
			: Thickness(thickness), Fade(fade), Color(color)
		{

		}
	};

	struct TextRendererComponent
	{
		UUID AssetHandle = 0;
		float KerningOffset = 0.0f; // Adjusts the spacing between characters proportionally
		float LineHeightOffset = 0.0f;
		std::string Text = std::string();
		glm::vec4 TextColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
		glm::vec4 BGColor = glm::vec4(0.0);

		float TextScalar = 1.0f;
		glm::vec2 TextOffset = glm::vec2(0.0f);

		TextRendererComponent() = default;
		TextRendererComponent(const TextRendererComponent&) = default;
	};

#pragma region GUI

	enum class GUIState
	{
		Disabled = 0, // Not taking Events
		Enabled, // Taking Events
		// Events
		Hovered, // Mouse
		Focused, // Keyboard
		// Event States
		Active, // TODO : Temp, will timeout
		Selected // Perm, needs to be unselected by user
	};
	struct GUIComponent
	{
		// Stores event
		GUIState LastState = GUIState::Disabled;
		// Handles immediate event
		GUIState CurrentState = GUIState::Enabled;

		bool IsNavigatable = true;

		GUIComponent() = default;
		GUIComponent(const GUIComponent&) = default;
	};

	enum class CanvasMode
	{
		None = 0, // UI will not render
		Overlay, // UI renders after every other game objects
		World, // UI may render before || after other game objects
	};
	struct GUICanvasComponent
	{
		bool ControlMouse = false;
		bool ShowMouse = true;
		CanvasMode Mode = CanvasMode::World;

		GUICanvasComponent() = default;
		GUICanvasComponent(const GUICanvasComponent&) = default;
	};

	enum class LayoutMode
	{
		Horizontal = 0,
		Vertical
	};
	struct GUILayoutComponent
	{
		LayoutMode Mode = LayoutMode::Horizontal;

		glm::vec2 StartingOffset = glm::vec2(0.0f);
		glm::vec2 ChildSize = glm::vec2(1.0f);
		glm::vec2 ChildPadding = glm::vec2(0.0f);

		const glm::vec2 GetEntityOffset();
	};

	struct GUIMaskComponent
	{
		// TODO :
	public:
		GUIMaskComponent() = default;
		GUIMaskComponent(const GUIMaskComponent&) = default;
	};

	struct GUIImageComponent
	{
		UUID TextureHandle = 0;
		glm::vec4 Color = glm::vec4(1.0f);
		float TilingFactor = 1.0f;
	public:
		GUIImageComponent() = default;
		GUIImageComponent(const GUIImageComponent&) = default;
	};

	struct GUIButtonComponent
	{
		// Font
		UUID FontAssetHandle = 0;
		float KerningOffset = 0.0f; // Adjusts the spacing between characters proportionally
		float LineHeightOffset = 0.0f;
		std::string Text = std::string();
		glm::vec4 TextColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
		glm::vec4 BGColor = glm::vec4(0.0);

		float TextScalar = 1.0f;
		glm::vec2 TextStartingOffset = glm::vec2(0.0f);
		glm::vec2 TextSize = glm::vec2(0.0f);

		// Background
		UUID BackgroundTextureHandle = 0;
		glm::vec4 BackgroundColor = glm::vec4(1.0f);

		// Middleground
		UUID DisabledTextureHandle = 0;
		glm::vec4 DisabledColor = glm::vec4(0.5f);

		UUID EnabledTextureHandle = 0;
		glm::vec4 EnabledColor = glm::vec4(1.0f);

		UUID HoveredTextureHandle = 0;
		glm::vec4 HoveredColor = glm::vec4(1.0f);

		UUID SelectedTextureHandle = 0;
		glm::vec4 SelectedColor = glm::vec4(0.5f);

		// Foreground
		UUID ForegroundTextureHandle = 0;
		glm::vec4 ForegroundColor = glm::vec4(0.0f);

		GUIButtonComponent() = default;
		GUIButtonComponent(const GUIButtonComponent&) = default;

	};
		
	struct GUIInputFieldComponent
	{
		UUID BackgroundTextureHandle = 0;
		glm::vec4 BackgroundColor = glm::vec4(1.0f);
		bool FillBackground = false;

		UUID FontAssetHandle = 0;
		float KerningOffset = 0.0f; // Adjusts the spacing between characters proportionally
		float LineHeightOffset = 0.0f;

		std::string Text = std::string();
		glm::vec4 BGColor = glm::vec4(0.0f);
		glm::vec4 TextColor = glm::vec4(0.0, 0.0, 0.0, 1.0);
		
		float TextScalar = 1.0f;
		glm::vec2 TextStartingOffset = glm::vec2(0.0f);

		glm::vec2 TextSize = glm::vec2(1.0f);
		glm::vec2 Padding = glm::vec2(0.25f);

		GUIInputFieldComponent() = default;
		GUIInputFieldComponent(const GUIInputFieldComponent&) = default;

	};

	enum class SliderDirection
	{
		None = 0,
		Left, // Horizontal Right->Left
		Center, // Horizontal Center->Right&&Left
		Right, // Horizontal Left->Right
		Top, // Vertical Bottom->Top
		Middle, // Vertical Middle->Bottom&&Top
		Bottom // Vertical Top->Bottom
	};

	struct GUISliderComponent
	{
		SliderDirection Direction = SliderDirection::Left;

		// Min: 0.0f, Max: 1.0f
		float Fill = 0.5f;

		UUID BackgroundTextureHandle = 0;
		glm::vec4 BackgroundColor = glm::vec4(1.0f);

		// Middleground
		UUID DisabledTextureHandle = 0;
		glm::vec4 DisabledColor = glm::vec4(0.5f);

		UUID EnabledTextureHandle = 0;
		glm::vec4 EnabledColor = glm::vec4(1.0f);

		UUID HoveredTextureHandle = 0;
		glm::vec4 HoveredColor = glm::vec4(1.0f);

		UUID SelectedTextureHandle = 0;
		glm::vec4 SelectedColor = glm::vec4(0.5f);

		UUID ForegroundTextureHandle = 0;
		glm::vec4 ForegroundColor = glm::vec4(0.0f);

	public:
		GUISliderComponent() = default;
		GUISliderComponent(const GUISliderComponent&) = default;

	};

	struct GUICheckboxComponent
	{
		UUID BackgroundTextureHandle = 0;
		glm::vec4 BackgroundColor = glm::vec4(1.0f);

		// Middleground
		UUID DisabledTextureHandle = 0;
		glm::vec4 DisabledColor = glm::vec4(0.5f);

		UUID EnabledTextureHandle = 0;
		glm::vec4 EnabledColor = glm::vec4(1.0f);

		UUID HoveredTextureHandle = 0;
		glm::vec4 HoveredColor = glm::vec4(1.0f);

		UUID SelectedTextureHandle = 0;
		glm::vec4 SelectedColor = glm::vec4(0.5f);

		UUID ForegroundTextureHandle = 0;
		glm::vec4 ForegroundColor = glm::vec4(0.0f);

		GUICheckboxComponent() = default;
		GUICheckboxComponent(const GUICheckboxComponent&) = default;

	};

	struct GUIScrollRectComponent
	{
		// TODO :
	public:
		GUIScrollRectComponent() = default;
		GUIScrollRectComponent(const GUIScrollRectComponent&) = default;
	};

	struct GUIScrollbarComponent
	{
		// TODO :
	public:
		GUIScrollbarComponent() = default;
		GUIScrollbarComponent(const GUIScrollbarComponent&) = default;
	};
#pragma endregion

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