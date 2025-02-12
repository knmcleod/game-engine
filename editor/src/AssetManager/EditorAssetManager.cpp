#include "EditorAssetManager.h"

#include "../Application/Layer/EditorLayer.h"
#include "../Application/Layer/EditorLayerStack.h"

#include <GE/GE.h>

#include <al.h>
#include <stb_image/stb_image.h>
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<GE::UUID>
	{
		static Node encode(const GE::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, GE::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow; // [0, 1, 2, ...]
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow; // [0, 1, 2, ...]
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow; // [0, 1, 2, ...]
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

}

namespace GE
{
#pragma region Entity
	
	static void SerializeEntity(YAML::Emitter& out, Ref<Scene> scene, const Entity& entity)
	{
		GE_CORE_ASSERT(scene->HasComponent<IDComponent>(entity), "Cannot serialize Entity without ID.");

		IDComponent& idc = scene->GetComponent<IDComponent>(entity);
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << idc.ID;
		if (scene->HasComponent<TagComponent>(entity))
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent
			auto& tc = scene->GetComponent<TagComponent>(entity);

			// TODO : Move TagStrs to EditorProject file. Base Project & Entity only needs the ID
			out << YAML::Key << "Tag" << YAML::Value << Project::GetStrByTag(tc.TagID).c_str();
			out << YAML::Key << "ID" << YAML::Value << tc.TagID;

			out << YAML::EndMap; // TagComponent
		}

		if (scene->HasComponent<NameComponent>(entity))
		{
			out << YAML::Key << "NameComponent";
			out << YAML::BeginMap; // NameComponent
			auto& name = scene->GetComponent<NameComponent>(entity).Name;
			GE_TRACE("UUID : {0},\n\tName : {1}", (uint64_t)idc.ID, name.c_str());

			out << YAML::Key << "Name" << YAML::Value << name;

			out << YAML::EndMap; // NameComponent
		}

		if (scene->HasComponent<ActiveComponent>(entity))
		{
			out << YAML::Key << "ActiveComponent";
			out << YAML::BeginMap; // ActiveComponent
			auto& component = scene->GetComponent<ActiveComponent>(entity);

			out << YAML::Key << "Active" << YAML::Value << component.Active;
			out << YAML::Key << "Hidden" << YAML::Value << component.Hidden;
	
			out << YAML::EndMap; // ActiveComponent
		}

		if (scene->HasComponent<RelationshipComponent>(entity))
		{
			out << YAML::Key << "RelationshipComponent";
			out << YAML::BeginMap; // RelationshipComponent
			auto& component = scene->GetComponent<RelationshipComponent>(entity);

			out << YAML::Key << "Parent" << YAML::Value << component.GetParent();
			if (component.GetChildren().size() > 0)
			{
				out << YAML::Key << "Children" << YAML::Value;
				out << YAML::BeginSeq; // Children
				for (const uint64_t& child : component.GetChildren())
				{
					out << YAML::Value << child;
				}
				out << YAML::EndSeq; // Children
			}
			out << YAML::EndMap; // RelationshipComponent
		}

		if (scene->HasComponent<TransformComponent>(entity))
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent
			auto& component = scene->GetComponent<TransformComponent>(entity);

			out << YAML::Key << "Translation" << YAML::Value << component.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << component.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << component.Scale;

			out << YAML::Key << "PivotEnum" << YAML::Value << component.GetPivot();

			out << YAML::EndMap; // TransformComponent
		}

		if (scene->HasComponent<AudioSourceComponent>(entity))
		{
			out << YAML::Key << "AudioSourceComponent";
			out << YAML::BeginMap; // AudioSourceComponent
			auto& component = scene->GetComponent<AudioSourceComponent>(entity);

			out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;
			out << YAML::Key << "Gain" << YAML::Value << component.Gain;
			out << YAML::Key << "Pitch" << YAML::Value << component.Pitch;
			out << YAML::Key << "Loop" << YAML::Value << component.Loop;

			out << YAML::EndMap; // AudioSourceComponent
		}

		if (scene->HasComponent<AudioListenerComponent>(entity))
		{
			out << YAML::Key << "AudioListenerComponent";
			out << YAML::BeginMap; // AudioListenerComponent
			auto& component = scene->GetComponent<AudioListenerComponent>(entity);

			out << YAML::EndMap; // AudioListenerComponent
		}

#pragma region Rendering

		if (scene->HasComponent<RenderComponent>(entity))
		{
			out << YAML::Key << "RenderComponent";
			out << YAML::BeginMap; // RenderComponent
			auto& rc = scene->GetComponent<RenderComponent>(entity);

			out << YAML::Key << "Layers" << YAML::Value;
			out << YAML::BeginSeq;
			Ref<EditorLayerStack> els = Application::GetLayerStack<EditorLayerStack>();
			for (uint64_t id : rc.LayerIDs)
			{
				if (Ref<Layer> layer = els->GetLayer(id))
				{
					out << YAML::BeginMap; // LayerID
					// TODO : Move name to EditorProject file. Entity only needs ID
					out << YAML::Key << "Name" << YAML::Value << els->GetLayerName(id).c_str();
					out << YAML::Key << "ID" << YAML::Value << layer->GetID();
					out << YAML::EndMap; // LayerID
				}
			}
			out << YAML::EndSeq;

			out << YAML::EndMap; // RenderComponent
		}

		if (scene->HasComponent<CameraComponent>(entity))
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent
			auto& component = scene->GetComponent<CameraComponent>(entity);
			auto& camera = component.ActiveCamera;

			out << YAML::Key << "FixedAspectRatio" << YAML::Value << component.FixedAspectRatio;
			out << YAML::Key << "Primary" << YAML::Value << component.Primary;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera

			out << YAML::Key << "Far" << YAML::Value << camera.GetFarClip();
			out << YAML::Key << "Near" << YAML::Value << camera.GetNearClip();
			out << YAML::Key << "FOV" << YAML::Value << camera.GetFOV();
			out << YAML::Key << "Type" << YAML::Value << (int)camera.GetProjectionType();

			out << YAML::EndMap; // Camera
			out << YAML::EndMap; // CameraComponent
		}

		if (scene->HasComponent<SpriteRendererComponent>(entity))
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent
			auto& component = scene->GetComponent<SpriteRendererComponent>(entity);
			out << YAML::Key << "Color" << YAML::Value << component.Color;
			if (component.AssetHandle)
			{
				out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;
			}

			if (component.TilingFactor)
			{
				out << YAML::Key << "TilingFactor" << YAML::Value << component.TilingFactor;
			}
			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (scene->HasComponent<CircleRendererComponent>(entity))
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent
			auto& component = scene->GetComponent<CircleRendererComponent>(entity);
			out << YAML::Key << "Color" << YAML::Value << component.Color;
			out << YAML::Key << "Radius" << YAML::Value << component.Radius;
			out << YAML::Key << "Thickness" << YAML::Value << component.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << component.Fade;
			out << YAML::EndMap; // CircleRendererComponent
		}

		if (scene->HasComponent<TextRendererComponent>(entity))
		{
			out << YAML::Key << "TextRendererComponent";
			out << YAML::BeginMap; // TextRendererComponent
			auto& component = scene->GetComponent<TextRendererComponent>(entity);
			out << YAML::Key << "TextColor" << YAML::Value << component.TextColor;
			out << YAML::Key << "BGColor" << YAML::Value << component.BGColor;

			out << YAML::Key << "KerningOffset" << YAML::Value << component.KerningOffset;
			out << YAML::Key << "LineHeightOffset" << YAML::Value << component.LineHeightOffset;
			out << YAML::Key << "Text" << YAML::Value << component.Text;

			out << YAML::Key << "TextScalar" << YAML::Value << component.TextScalar;
			out << YAML::Key << "TextOffset" << YAML::Value << component.TextOffset;

			out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;
			out << YAML::EndMap; // TextRendererComponent
		}

		if (scene->HasComponent<GUICanvasComponent>(entity))
		{
			out << YAML::Key << "GUICanvasComponent";
			out << YAML::BeginMap; // GUICanvasComponent
			auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
			
			out << YAML::Key << "ControlMouse" << YAML::Value << guiCC.ControlMouse;
			out << YAML::Key << "ShowMouse" << YAML::Value << guiCC.ShowMouse;

			out << YAML::Key << "Mode" << YAML::Value << (uint32_t)guiCC.Mode;

			out << YAML::EndMap; // GUICanvasComponent
		}

		if (scene->HasComponent<GUILayoutComponent>(entity))
		{
			out << YAML::Key << "GUILayoutComponent";
			out << YAML::BeginMap; // GUILayoutComponent
			auto& guiLOC = scene->GetComponent<GUILayoutComponent>(entity);

			out << YAML::Key << "Mode" << YAML::Value << (uint32_t)guiLOC.Mode;
			out << YAML::Key << "StartingOffset" << YAML::Value << guiLOC.StartingOffset;
			out << YAML::Key << "ChildSize" << YAML::Value << guiLOC.ChildSize;
			out << YAML::Key << "ChildPadding" << YAML::Value << guiLOC.ChildPadding;

			out << YAML::EndMap; // GUILayoutComponent
		}
		// TODO : GUIMaskComponent

		if (scene->HasComponent<GUIImageComponent>(entity))
		{
			out << YAML::Key << "GUIImageComponent";
			out << YAML::BeginMap; // GUIImageComponent
			auto& guiIC = scene->GetComponent<GUIImageComponent>(entity);
			out << YAML::Key << "Color" << YAML::Value << guiIC.Color;
			out << YAML::Key << "TextureHandle" << YAML::Value << guiIC.TextureHandle;
			out << YAML::Key << "TilingFactor" << YAML::Value << guiIC.TilingFactor;

			out << YAML::EndMap; // GUIImageComponent

		}
		if (scene->HasComponent<GUIButtonComponent>(entity))
		{
			out << YAML::Key << "GUIButtonComponent";
			out << YAML::BeginMap; // GUIButtonComponent
			auto& guiBC = scene->GetComponent<GUIButtonComponent>(entity);
			out << YAML::Key << "TextColor" << YAML::Value << guiBC.TextColor;
			out << YAML::Key << "BGColor" << YAML::Value << guiBC.BGColor;

			out << YAML::Key << "KerningOffset" << YAML::Value << guiBC.KerningOffset;
			out << YAML::Key << "LineHeightOffset" << YAML::Value << guiBC.LineHeightOffset;
			out << YAML::Key << "Text" << YAML::Value << guiBC.Text;

			out << YAML::Key << "FontAssetHandle" << YAML::Value << guiBC.FontAssetHandle;

			out << YAML::Key << "BackgroundTextureHandle" << YAML::Value << guiBC.BackgroundTextureHandle;
			out << YAML::Key << "BackgroundColor" << YAML::Value << guiBC.BackgroundColor;

			out << YAML::Key << "DisabledColor" << YAML::Value << guiBC.DisabledColor;
			out << YAML::Key << "DisabledTextureHandle" << YAML::Value << guiBC.DisabledTextureHandle;

			out << YAML::Key << "EnabledColor" << YAML::Value << guiBC.EnabledColor;
			out << YAML::Key << "EnabledTextureHandle" << YAML::Value << guiBC.EnabledTextureHandle;

			out << YAML::Key << "HoveredColor" << YAML::Value << guiBC.HoveredColor;
			out << YAML::Key << "HoveredTextureHandle" << YAML::Value << guiBC.HoveredTextureHandle;

			out << YAML::Key << "SelectedColor" << YAML::Value << guiBC.SelectedColor;
			out << YAML::Key << "SelectedTextureHandle" << YAML::Value << guiBC.SelectedTextureHandle;

			out << YAML::Key << "ForegroundTextureHandle" << YAML::Value << guiBC.ForegroundTextureHandle;
			out << YAML::Key << "ForegroundColor" << YAML::Value << guiBC.ForegroundColor;

			out << YAML::EndMap; // GUIButtonComponent
		}

		if (scene->HasComponent<GUIInputFieldComponent>(entity))
		{
			out << YAML::Key << "GUIInputFieldComponent";
			out << YAML::BeginMap; // GUIInputFieldComponent
			auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);

			out << YAML::Key << "BackgroundColor" << YAML::Value << guiIFC.BackgroundColor;
			out << YAML::Key << "BackgroundTextureHandle" << YAML::Value << guiIFC.BackgroundTextureHandle;

			out << YAML::Key << "FillBackground" << YAML::Value << guiIFC.FillBackground;
			out << YAML::Key << "TextSize" << YAML::Value << guiIFC.TextSize;

			out << YAML::Key << "TextColor" << YAML::Value << guiIFC.TextColor;
			out << YAML::Key << "BGColor" << YAML::Value << guiIFC.BGColor;

			out << YAML::Key << "FontAssetHandle" << YAML::Value << guiIFC.FontAssetHandle;

			out << YAML::Key << "KerningOffset" << YAML::Value << guiIFC.KerningOffset;
			out << YAML::Key << "LineHeightOffset" << YAML::Value << guiIFC.LineHeightOffset;
			out << YAML::Key << "Text" << YAML::Value << guiIFC.Text;

			out << YAML::Key << "TextScalar" << YAML::Value << guiIFC.TextScalar;
			out << YAML::Key << "TextOffset" << YAML::Value << guiIFC.TextStartingOffset;

			out << YAML::Key << "Padding" << YAML::Value << guiIFC.Padding;

			out << YAML::EndMap; // GUIInputFieldComponent
		}

		if (scene->HasComponent<GUISliderComponent>(entity))
		{
			out << YAML::Key << "GUISliderComponent";
			out << YAML::BeginMap; // GUISliderComponent
			auto& guiSC = scene->GetComponent<GUISliderComponent>(entity);

			out << YAML::Key << "Direction" << YAML::Value << EditorAssetManager::SliderDirectionToString(guiSC.Direction);
			out << YAML::Key << "Fill" << YAML::Value << guiSC.Fill;

			out << YAML::Key << "BackgroundColor" << YAML::Value << guiSC.BackgroundColor;
			out << YAML::Key << "BackgroundTextureHandle" << YAML::Value << guiSC.BackgroundTextureHandle;

			out << YAML::Key << "DisabledColor" << YAML::Value << guiSC.DisabledColor;
			out << YAML::Key << "DisabledTextureHandle" << YAML::Value << guiSC.DisabledTextureHandle;

			out << YAML::Key << "EnabledColor" << YAML::Value << guiSC.EnabledColor;
			out << YAML::Key << "EnabledTextureHandle" << YAML::Value << guiSC.EnabledTextureHandle;

			out << YAML::Key << "HoveredColor" << YAML::Value << guiSC.HoveredColor;
			out << YAML::Key << "HoveredTextureHandle" << YAML::Value << guiSC.HoveredTextureHandle;

			out << YAML::Key << "SelectedColor" << YAML::Value << guiSC.SelectedColor;
			out << YAML::Key << "SelectedTextureHandle" << YAML::Value << guiSC.SelectedTextureHandle;

			out << YAML::Key << "ForegroundColor" << YAML::Value << guiSC.ForegroundColor;
			out << YAML::Key << "ForegroundTextureHandle" << YAML::Value << guiSC.ForegroundTextureHandle;

			out << YAML::EndMap; // GUISliderComponent
		}
		
		if (scene->HasComponent<GUICheckboxComponent>(entity))
		{
			out << YAML::Key << "GUICheckboxComponent";
			out << YAML::BeginMap; // GUICheckboxComponent
			auto& guiCB = scene->GetComponent<GUICheckboxComponent>(entity);

			out << YAML::Key << "BackgroundColor" << YAML::Value << guiCB.BackgroundColor;
			out << YAML::Key << "BackgroundTextureHandle" << YAML::Value << guiCB.BackgroundTextureHandle;

			out << YAML::Key << "DisabledColor" << YAML::Value << guiCB.DisabledColor;
			out << YAML::Key << "DisabledTextureHandle" << YAML::Value << guiCB.DisabledTextureHandle;

			out << YAML::Key << "EnabledColor" << YAML::Value << guiCB.EnabledColor;
			out << YAML::Key << "EnabledTextureHandle" << YAML::Value << guiCB.EnabledTextureHandle;

			out << YAML::Key << "HoveredColor" << YAML::Value << guiCB.HoveredColor;
			out << YAML::Key << "HoveredTextureHandle" << YAML::Value << guiCB.HoveredTextureHandle;

			out << YAML::Key << "SelectedColor" << YAML::Value << guiCB.SelectedColor;
			out << YAML::Key << "SelectedTextureHandle" << YAML::Value << guiCB.SelectedTextureHandle;

			out << YAML::Key << "ForegroundColor" << YAML::Value << guiCB.ForegroundColor;
			out << YAML::Key << "ForegroundTextureHandle" << YAML::Value << guiCB.ForegroundTextureHandle;

			out << YAML::EndMap; // GUICheckboxComponent
		}

		// TODO : GUIScrollRectComponent & GUIScrollbarComponent 
#pragma endregion

#pragma region Scripting

		if (scene->HasComponent<NativeScriptComponent>(entity))
		{
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap; // NativeScriptComponent
			auto& component = scene->GetComponent<NativeScriptComponent>(entity);

			out << YAML::EndMap; // NativeScriptComponent
		}

		if (scene->HasComponent<ScriptComponent>(entity))
		{
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent
			auto& component = scene->GetComponent<ScriptComponent>(entity);

			out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;

			// Fields
			if (Scripting::ScriptExists(component.AssetHandle))
			{
				Ref<Script> script = Scripting::GetScript(component.AssetHandle);
				ScriptFieldMap& instanceFields = Scripting::GetEntityFields(scene->GetComponent<IDComponent>(entity).ID);
				if (!instanceFields.empty())
				{
					out << YAML::Key << "ScriptFields" << YAML::Value;
					out << YAML::BeginSeq;

					for (const auto& [name, instanceField] : instanceFields)
					{
						if (!script->FieldExists(name))
							continue;

						ScriptField::Type type = instanceField.GetType();
						out << YAML::BeginMap; // InstanceScriptFields

						out << YAML::Key << "Name" << YAML::Value << name;
						out << YAML::Key << "Type" << YAML::Value << Scripting::ScriptFieldTypeToString(type);
						out << YAML::Key << "Data" << YAML::Value;

						switch (type)
						{
						case ScriptField::Type::Char:
							out << instanceField.GetValue<char>();
							break;
						case ScriptField::Type::Int:
							out << instanceField.GetValue<int>();
							break;
						case ScriptField::Type::UInt:
							out << instanceField.GetValue<uint32_t>();
							break;
						case ScriptField::Type::Float:
							out << instanceField.GetValue<float>();
							break;
						case ScriptField::Type::Byte:
							out << instanceField.GetValue<int8_t>();
							break;
						case ScriptField::Type::Bool:
							out << instanceField.GetValue<bool>();
							break;
						case ScriptField::Type::Vector2:
							out << instanceField.GetValue<glm::vec2>();
							break;
						case ScriptField::Type::Vector3:
							out << instanceField.GetValue<glm::vec3>();
							break;
						case ScriptField::Type::Vector4:
							out << instanceField.GetValue<glm::vec4>();
							break;
						case ScriptField::Type::Entity:
							out << instanceField.GetValue<UUID>();
							break;
						}

						out << YAML::EndMap; // InstanceScriptFields
					}
					out << YAML::EndSeq;
				}
			}
			out << YAML::EndMap; // ScriptComponent
		}
#pragma endregion

#pragma region Physics

		if (scene->HasComponent<Rigidbody2DComponent>(entity))
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent
			auto& component = scene->GetComponent<Rigidbody2DComponent>(entity);
			const std::string typeString = ComponentUtils::GetStringFromRigidBody2DType(component.Type);
			out << YAML::Key << "Type" << YAML::Value << typeString;
			out << YAML::Key << "FixedRotation" << YAML::Value << component.FixedRotation;
			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (scene->HasComponent<BoxCollider2DComponent>(entity))
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent
			auto& component = scene->GetComponent<BoxCollider2DComponent>(entity);
			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "ChildSize" << YAML::Value << component.Size;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;
			out << YAML::Key << "Show" << YAML::Value << component.Show;
			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (scene->HasComponent<CircleCollider2DComponent>(entity))
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent
			auto& component = scene->GetComponent<CircleCollider2DComponent>(entity);
			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "Radius" << YAML::Value << component.Radius;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;
			out << YAML::Key << "Show" << YAML::Value << component.Show;
			out << YAML::EndMap; // CircleCollider2DComponent
		}
#pragma endregion

		out << YAML::EndMap; // Entity
	}

	static bool DeserializeEntity(Ref<Scene> scene, const YAML::detail::iterator_value& eDetails, Entity& entity)
	{
		// ActiveComponent
		if (auto& activeComponent = eDetails["ActiveComponent"])
		{
			auto& ac = scene->GetOrAddComponent<ActiveComponent>(entity);
			if(auto& active = activeComponent["Active"])
				ac.Active = active.as<bool>();
			if (auto& hidden = activeComponent["Hidden"])
				ac.Hidden = hidden.as<bool>();
		}

		// RelationshipComponent
		if (auto& relationshipComponent = eDetails["RelationshipComponent"])
		{
			auto& rsc = scene->GetOrAddComponent<RelationshipComponent>(entity);
			if (auto& parent = relationshipComponent["Parent"])
			{
				uint64_t parentID = parent.as<uint64_t>();
				std::vector<UUID> entityChildren = std::vector<UUID>();
				if (auto& children = relationshipComponent["Children"])
				{
					for (auto& child : children)
					{
						entityChildren.push_back(child.as<UUID>());
					}
				}
				
				rsc = RelationshipComponent(parentID, entityChildren);
				entityChildren.clear();
				entityChildren = std::vector<UUID>();
			}
		}
		// TransformComponent
		if (auto& transformComponent = eDetails["TransformComponent"])
		{
			auto& trsc = scene->GetOrAddComponent<TransformComponent>(entity);
			if(auto& translation = transformComponent["Translation"])
				trsc.Translation = translation.as<glm::vec3>();
			if (auto& rotation = transformComponent["Rotation"])
				trsc.Rotation = rotation.as<glm::vec3>();
			if (auto& scale = transformComponent["Scale"])
				trsc.Scale = scale.as<glm::vec3>();
			if(auto& pivot = transformComponent["PivotEnum"])
				trsc.SetPivot((Pivot)pivot.as<uint32_t>());
		}

		// AudioSourceComponent
		if (auto& audioSourceComponent = eDetails["AudioSourceComponent"])
		{
			auto& asc = scene->GetOrAddComponent<AudioSourceComponent>(entity);
			if(auto& handle = audioSourceComponent["AssetHandle"])
				asc.AssetHandle = handle.as<UUID>();
			if (auto& gain = audioSourceComponent["Gain"])
				asc.Gain = gain.as<float>();
			if (auto& handle = audioSourceComponent["Pitch"])
				asc.Pitch = handle.as<float>();
			if (auto& handle = audioSourceComponent["Loop"])
				asc.Loop = handle.as<bool>();
		}

		// AudioListenerComponent
		if (auto& audioListenerComponent = eDetails["AudioListenerComponent"])
		{
			auto& cc = scene->GetOrAddComponent<AudioListenerComponent>(entity);

		}
#pragma region Rendering
		// RenderComponent
		if (auto renderComponent = eDetails["RenderComponent"])
		{
			auto& rc = scene->GetOrAddComponent<RenderComponent>(entity);

			if (auto& layers = renderComponent["Layers"])
			{
				Ref<EditorLayerStack> els = Application::GetLayerStack<EditorLayerStack>();
				for (auto& layer : layers)
				{
					std::string layerName = std::string();
					uint64_t layerID = 0;

					// TODO : Move to EditorProject Serialization
					// Base Project will only serialize the ID
					// Entity Component will only serialize the ID
					if(auto& name = layer["Name"])
						layerName = name.as<std::string>();
					if(auto& id = layer["ID"])
						layerID = id.as<uint64_t>();

					rc.AddID(layerID);

					if (els)
					{
						if (!els->LayerExists(layerID) || !els->LayerNameExists(layerName))
						{
							const Layer::Type& layerType = EditorAssetManager::GetTypeFromName(layerName);
							Ref<Layer> newLayer = nullptr;
							switch (layerType)
							{
							case Layer::Type::Debug:
							{
								newLayer = CreateRef<EditorLayer>(layerID);
							}
							break;
							case Layer::Type::Layer:
							{
								newLayer = CreateRef<Layer>(layerID);
							}
							break;
							case Layer::Type::GUI:
							{
								newLayer = CreateRef<GUILayer>(layerID);
							}
							break;
							default:
								GE_WARN("Unknown Layer::Type.");
								break;
							}

							if (els->InsertLayer(newLayer, layerName))
								GE_TRACE("Layer Added - {0}, {1}", layerName.c_str(), layerID);
						}
					}

				}
			}
		}

		// CameraComponent
		if (auto& cameraComponent = eDetails["CameraComponent"])
		{
			auto& cc = scene->GetOrAddComponent<CameraComponent>(entity);
			if (auto& cameraProps = cameraComponent["Camera"])
			{
				if(auto& projectionType = cameraProps["Type"])
					cc.SetProjectionType((SceneCamera::ProjectionType)projectionType.as<int>());
				if(auto& fov = cameraProps["FOV"])
					cc.SetFOV(fov.as<float>());
				if (auto& nearclip = cameraProps["Near"])
					cc.SetNearClip(nearclip.as<float>());
				if (auto& farclip = cameraProps["Far"])
					cc.SetFarClip(farclip.as<float>());

				if(auto& isPrimary = cameraComponent["Primary"])
					cc.Primary = isPrimary.as<bool>();
				if (auto& fixedAspectRatio = cameraComponent["FixedAspectRatio"])
					cc.FixedAspectRatio = fixedAspectRatio.as<bool>();
			}
		}

		// SpriteRendererComponent
		if (auto& spriteRendererComponent = eDetails["SpriteRendererComponent"])
		{
			auto& src = scene->GetOrAddComponent<SpriteRendererComponent>(entity);
			if (auto& color = spriteRendererComponent["Color"])
				src.Color = color.as<glm::vec4>();
			if (auto& handle = spriteRendererComponent["AssetHandle"])
				src.AssetHandle = handle.as<UUID>();

			if (auto& tilingFactor = spriteRendererComponent["TilingFactor"])
				src.TilingFactor = tilingFactor.as<float>();
		}

		// CircleRendererComponent
		if (auto& circleRendererComponent = eDetails["CircleRendererComponent"])
		{
			auto& src = scene->GetOrAddComponent<CircleRendererComponent>(entity);
			if(auto& color = circleRendererComponent["Color"])
				src.Color = color.as<glm::vec4>();
			if (auto& radius = circleRendererComponent["Radius"])
				src.Radius = radius.as<float>();
			if (auto& thickness = circleRendererComponent["Thickness"])
				src.Thickness = thickness.as<float>();
			if (auto& fade = circleRendererComponent["Fade"])
				src.Fade = fade.as<float>();
		}

		// TextRendererComponent
		if (auto& textRendererComponent = eDetails["TextRendererComponent"])
		{
			auto& trc = scene->GetOrAddComponent<TextRendererComponent>(entity);

			if (auto& textColor = textRendererComponent["TextColor"])
				trc.TextColor = textColor.as<glm::vec4>();
			if (auto& bgColor = textRendererComponent["BGColor"])
				trc.BGColor = bgColor.as<glm::vec4>();

			if (auto& xOffset = textRendererComponent["KerningOffset"])
				trc.KerningOffset = xOffset.as<float>();
			if (auto& yOffset = textRendererComponent["LineHeightOffset"])
				trc.LineHeightOffset = yOffset.as<float>();

			if (auto& text = textRendererComponent["Text"])
				trc.Text = text.as<std::string>();

			if (auto& scalar = textRendererComponent["TextScalar"])
				trc.TextScalar = scalar.as<float>();
			if (auto& offset = textRendererComponent["TextOffset"])
				trc.TextOffset = offset.as<glm::vec2>();
			if(auto& handle = textRendererComponent["AssetHandle"])
				trc.AssetHandle = handle.as<UUID>();
		}

#pragma region UI

		if (auto& guiCanvasComponent = eDetails["GUICanvasComponent"])
		{
			auto& guiCC = scene->GetOrAddComponent<GUICanvasComponent>(entity);
			if (auto& control = guiCanvasComponent["ControlMouse"])
				guiCC.ControlMouse = control.as<bool>();
			if (auto& show = guiCanvasComponent["ShowMouse"])
				guiCC.ShowMouse = show.as<bool>();
			if(auto& mode = guiCanvasComponent["Mode"])
				guiCC.Mode = (CanvasMode)mode.as<uint32_t>();
		}

		if (auto& guiLayoutComponent = eDetails["GUILayoutComponent"])
		{
			auto& guiLOC = scene->GetOrAddComponent<GUILayoutComponent>(entity);
			if(auto& mode = guiLayoutComponent["Mode"])
				guiLOC.Mode = (LayoutMode)mode.as<uint32_t>();
			if(auto& offset = guiLayoutComponent["StartingOffset"])
				guiLOC.StartingOffset = offset.as<glm::vec2>();
			if(auto& size = guiLayoutComponent["ChildSize"])
				guiLOC.ChildSize = size.as<glm::vec2>();
			if (auto& padding = guiLayoutComponent["ChildPadding"])
				guiLOC.ChildPadding = padding.as<glm::vec2>();
		}

		// TODO : GUIMaskComponent

		if (auto& guiImageComponent = eDetails["GUIImageComponent"])
		{
			auto& guiIC = scene->GetOrAddComponent<GUIImageComponent>(entity);

			if (auto& color = guiImageComponent["Color"])
				guiIC.Color = color.as<glm::vec4>();
			if (auto& handle = guiImageComponent["TextureHandle"])
				guiIC.TextureHandle = handle.as<UUID>();
			if (auto& tilingFactor = guiImageComponent["TilingFactor"])
				guiIC.TilingFactor = tilingFactor.as<float>();

		}

		if (auto& guiButtonComponent = eDetails["GUIButtonComponent"])
		{
			auto& guiBC = scene->GetOrAddComponent<GUIButtonComponent>(entity);

			if (auto& fHandle = guiButtonComponent["FontAssetHandle"])
				guiBC.FontAssetHandle = fHandle.as<UUID>();

			if (auto& scalar = guiButtonComponent["KerningOffset"])
				guiBC.KerningOffset = scalar.as<float>();
			if (auto& scalar = guiButtonComponent["LineHeightOffset"])
				guiBC.LineHeightOffset = scalar.as<float>();

			if (auto& text = guiButtonComponent["Text"])
				guiBC.Text = text.as<std::string>();

			if (auto& textColor = guiButtonComponent["TextColor"])
				guiBC.TextColor = textColor.as<glm::vec4>();
			if (auto& scalar = guiButtonComponent["BGColor"])
				guiBC.BGColor = scalar.as<glm::vec4>();

			if (auto& scalar = guiButtonComponent["TextScalar"])
				guiBC.TextScalar = scalar.as<float>();
			if (auto& offset = guiButtonComponent["TextOffset"])
				guiBC.TextStartingOffset = offset.as<glm::vec2>();

			if (auto& bgScalar = guiButtonComponent["TextSize"])
				guiBC.TextSize = bgScalar.as<glm::vec2>();

			if (auto& bgColor = guiButtonComponent["BackgroundColor"])
				guiBC.BackgroundColor = bgColor.as<glm::vec4>();
			if (auto& bgHandle = guiButtonComponent["BackgroundTextureHandle"])
				guiBC.BackgroundTextureHandle = bgHandle.as<UUID>();

			if (auto& iac = guiButtonComponent["DisabledColor"])
				guiBC.DisabledColor = iac.as<glm::vec4>();
			if (auto& iaHandle = guiButtonComponent["DisabledTextureHandle"])
				guiBC.DisabledTextureHandle = iaHandle.as<UUID>();

			if (auto& ac = guiButtonComponent["EnabledColor"])
				guiBC.EnabledColor = ac.as<glm::vec4>();
			if(auto& aHandle = guiButtonComponent["EnabledTextureHandle"])
				guiBC.EnabledTextureHandle = aHandle.as<UUID>();

			if (auto& hv = guiButtonComponent["HoveredColor"])
				guiBC.HoveredColor = hv.as<glm::vec4>();
			if (auto& sHandle = guiButtonComponent["HoveredTextureHandle"])
				guiBC.HoveredTextureHandle = sHandle.as<UUID>();

			if (auto& sc = guiButtonComponent["SelectedColor"])
				guiBC.SelectedColor = sc.as<glm::vec4>();
			if (auto& sHandle = guiButtonComponent["SelectedTextureHandle"])
				guiBC.SelectedTextureHandle = sHandle.as<UUID>();

			if (auto& fgColor = guiButtonComponent["ForegroundColor"])
				guiBC.ForegroundColor = fgColor.as<glm::vec4>();
			if (auto& fgHandle = guiButtonComponent["ForegroundTextureHandle"])
				guiBC.ForegroundTextureHandle = fgHandle.as<UUID>();

		}

		if (auto& guiInputFieldComponent = eDetails["GUIInputFieldComponent"])
		{
			auto& guiIFC = scene->GetOrAddComponent<GUIInputFieldComponent>(entity);

			if(auto& bgColor = guiInputFieldComponent["BackgroundColor"])
				guiIFC.BackgroundColor = bgColor.as<glm::vec4>();
			if (auto& bgHandle = guiInputFieldComponent["BackgroundTextureHandle"])
				guiIFC.BackgroundTextureHandle = bgHandle.as<UUID>();

			if (auto& fillBG = guiInputFieldComponent["FillBackground"])
				guiIFC.FillBackground = fillBG.as<bool>();

			if (auto& fontHandle = guiInputFieldComponent["FontAssetHandle"])
				guiIFC.FontAssetHandle = fontHandle.as<UUID>();

			if (auto& xOffset = guiInputFieldComponent["KerningOffset"])
				guiIFC.KerningOffset = xOffset.as<float>();
			if (auto& yOffset = guiInputFieldComponent["LineHeightOffset"])
				guiIFC.LineHeightOffset = yOffset.as<float>();

			if(auto& input = guiInputFieldComponent["Text"])
				guiIFC.Text = input.as<std::string>();

			if (auto& textColor = guiInputFieldComponent["TextColor"])
				guiIFC.TextColor = textColor.as<glm::vec4>();
			if (auto& bgColor = guiInputFieldComponent["BGColor"])
				guiIFC.BGColor = bgColor.as<glm::vec4>();

			if (auto& scalar = guiInputFieldComponent["TextScalar"])
				guiIFC.TextScalar = scalar.as<float>();
			if (auto& offset = guiInputFieldComponent["TextOffset"])
				guiIFC.TextStartingOffset = offset.as<glm::vec2>();

			if (auto& padding = guiInputFieldComponent["Padding"])
				guiIFC.Padding = padding.as<glm::vec2>();

		}

		if (auto& guiSliderComponent = eDetails["GUISliderComponent"])
		{
			auto& guiSC = scene->GetOrAddComponent<GUISliderComponent>(entity);

			if (auto& direction = guiSliderComponent["Direction"])
			{
				std::string dirStr = direction.as<std::string>();
				guiSC.Direction = EditorAssetManager::StringToSliderDirection(dirStr);
			}
			if (auto& current = guiSliderComponent["Fill"])
				guiSC.Fill = current.as<float>();

			if (auto& bgColor = guiSliderComponent["BackgroundColor"])
				guiSC.BackgroundColor = bgColor.as<glm::vec4>();
			if (auto& bgHandle = guiSliderComponent["BackgroundTextureHandle"])
				guiSC.BackgroundTextureHandle = bgHandle.as<UUID>();

			if (auto& iac = guiSliderComponent["DisabledColor"])
				guiSC.DisabledColor = iac.as<glm::vec4>();
			if (auto& iaHandle = guiSliderComponent["DisabledTextureHandle"])
				guiSC.DisabledTextureHandle = iaHandle.as<UUID>();

			if (auto& ac = guiSliderComponent["EnabledColor"])
				guiSC.EnabledColor = ac.as<glm::vec4>();
			if (auto& aHandle = guiSliderComponent["EnabledTextureHandle"])
				guiSC.EnabledTextureHandle = aHandle.as<UUID>();

			if (auto& hv = guiSliderComponent["HoveredColor"])
				guiSC.HoveredColor = hv.as<glm::vec4>();
			if (auto& sHandle = guiSliderComponent["HoveredTextureHandle"])
				guiSC.HoveredTextureHandle = sHandle.as<UUID>();

			if (auto& sc = guiSliderComponent["SelectedColor"])
				guiSC.SelectedColor = sc.as<glm::vec4>();
			if (auto& sHandle = guiSliderComponent["SelectedTextureHandle"])
				guiSC.SelectedTextureHandle = sHandle.as<UUID>();

			if (auto& fgColor = guiSliderComponent["ForegroundColor"])
				guiSC.ForegroundColor = fgColor.as<glm::vec4>();
			if (auto& fgHandle = guiSliderComponent["ForegroundTextureHandle"])
				guiSC.ForegroundTextureHandle = fgHandle.as<UUID>();

		}

		if (auto& guiCheckboxComponent = eDetails["GUICheckboxComponent"])
		{
			auto& guiSC = scene->GetOrAddComponent<GUICheckboxComponent>(entity);

			if (auto& bgColor = guiCheckboxComponent["BackgroundColor"])
				guiSC.BackgroundColor = bgColor.as<glm::vec4>();
			if (auto& bgHandle = guiCheckboxComponent["BackgroundTextureHandle"])
				guiSC.BackgroundTextureHandle = bgHandle.as<UUID>();
			
			if (auto& iac = guiCheckboxComponent["DisabledColor"])
				guiSC.DisabledColor = iac.as<glm::vec4>();
			if (auto& iaHandle = guiCheckboxComponent["DisabledTextureHandle"])
				guiSC.DisabledTextureHandle = iaHandle.as<UUID>();

			if (auto& ac = guiCheckboxComponent["EnabledColor"])
				guiSC.EnabledColor = ac.as<glm::vec4>();
			if (auto& aHandle = guiCheckboxComponent["EnabledTextureHandle"])
				guiSC.EnabledTextureHandle = aHandle.as<UUID>();

			if (auto& hv = guiCheckboxComponent["HoveredColor"])
				guiSC.HoveredColor = hv.as<glm::vec4>();
			if (auto& sHandle = guiCheckboxComponent["HoveredTextureHandle"])
				guiSC.HoveredTextureHandle = sHandle.as<UUID>();

			if (auto& sc = guiCheckboxComponent["SelectedColor"])
				guiSC.SelectedColor = sc.as<glm::vec4>();
			if (auto& sHandle = guiCheckboxComponent["SelectedTextureHandle"])
				guiSC.SelectedTextureHandle = sHandle.as<UUID>();

			if (auto& fgColor = guiCheckboxComponent["ForegroundColor"])
				guiSC.ForegroundColor = fgColor.as<glm::vec4>();
			if (auto& fgHandle = guiCheckboxComponent["ForegroundTextureHandle"])
				guiSC.ForegroundTextureHandle = fgHandle.as<UUID>();

		}

		// TODO : GUIScrollRectComponent & GUIScrollbarComponent
#pragma endregion


#pragma endregion

#pragma region Scripting
		// NativeScriptComponent
		if (auto& nativeScriptComponent = eDetails["NativeScriptComponent"])
		{
			auto& nsc = scene->GetOrAddComponent<NativeScriptComponent>(entity);
		}

		// ScriptComponent
		if (auto& scriptComponent = eDetails["ScriptComponent"])
		{
			auto& sc = scene->GetOrAddComponent<ScriptComponent>(entity);

			if(auto& handle = scriptComponent["AssetHandle"])
				sc.AssetHandle = handle.as<uint64_t>();

			if (auto& fields = scriptComponent["ScriptFields"])
			{
				auto& deserializedInstanceFields = Scripting::GetEntityFields(scene->GetComponent<IDComponent>(entity).ID);

				for (auto& field : fields)
				{
					std::string scriptFieldName = std::string();
					std::string scriptFieldTypeStr = std::string();
					if(auto& fieldName = field["Name"])
						scriptFieldName = fieldName.as<std::string>();
					if (auto& fieldTypeStr = field["Type"])
						scriptFieldTypeStr = fieldTypeStr.as<std::string>();

					ScriptField::Type scriptFieldType = Scripting::StringToScriptFieldType(scriptFieldTypeStr);

					ScriptField& instanceScriptField = deserializedInstanceFields[scriptFieldName];
					instanceScriptField = ScriptField(scriptFieldTypeStr, scriptFieldType, nullptr);
					if (auto& fieldData = field["Data"])
					{
						switch (scriptFieldType)
						{
						case GE::ScriptField::Type::None:
							break;
						case GE::ScriptField::Type::Char:
						{
							char data = fieldData.as<char>();
							instanceScriptField.SetValue<char>(data);
							break;
						}
						case GE::ScriptField::Type::Int:
						{
							int32_t data = fieldData.as<int32_t>();
							instanceScriptField.SetValue<int32_t>(data);
							break;
						}
						case GE::ScriptField::Type::UInt:
						{
							uint32_t data = fieldData.as<uint32_t>();
							instanceScriptField.SetValue<uint32_t>(data);
							break;
						}
						case GE::ScriptField::Type::Float:
						{
							float data = fieldData.as<float>();
							instanceScriptField.SetValue<float>(data);
							break;
						}
						case GE::ScriptField::Type::Byte:
						{
							int8_t data = fieldData.as<int8_t>();
							instanceScriptField.SetValue<int8_t>(data);
							break;
						}
						case GE::ScriptField::Type::Bool:
						{
							bool data = fieldData.as<bool>();
							instanceScriptField.SetValue<bool>(data);
							break;
						}
						case GE::ScriptField::Type::Vector2:
						{
							glm::vec2 data = fieldData.as<glm::vec2>();
							instanceScriptField.SetValue<glm::vec2>(data);
							break;
						}
						case GE::ScriptField::Type::Vector3:
						{
							glm::vec3 data = fieldData.as<glm::vec3>();
							instanceScriptField.SetValue<glm::vec3>(data);
							break;
						}
						case GE::ScriptField::Type::Vector4:
						{
							glm::vec4 data = fieldData.as<glm::vec4>();
							instanceScriptField.SetValue<glm::vec4>(data);
							break;
						}
						case GE::ScriptField::Type::Entity:
						{
							UUID data = fieldData.as<UUID>();
							instanceScriptField.SetValue<UUID>(data);
							break;
						}
						}
					}
				}
			}

		}

#pragma endregion

#pragma region Physics

		// RigidBody2DComponent
		if (auto& rigidBody2DComponent = eDetails["Rigidbody2DComponent"])
		{
			auto& rb2DC = scene->GetOrAddComponent<Rigidbody2DComponent>(entity);
			if(auto& fixedRotation = rigidBody2DComponent["FixedRotation"])
				rb2DC.FixedRotation = fixedRotation.as<bool>();
			if(auto& type = rigidBody2DComponent["Type"])
				rb2DC.Type = ComponentUtils::GetRigidBody2DTypeFromString(type.as<std::string>());
		}

		// BoxCollider2DComponent
		if (auto& boxCollider2DComponent = eDetails["BoxCollider2DComponent"])
		{
			auto& bc2DC = scene->GetOrAddComponent<BoxCollider2DComponent>(entity);
	
			if (auto& offset = boxCollider2DComponent["Offset"])
				bc2DC.Offset = offset.as<glm::vec2>();
			if (auto& size = boxCollider2DComponent["ChildSize"])
				bc2DC.Size = size.as<glm::vec2>();
			if (auto& density = boxCollider2DComponent["Density"])
				bc2DC.Density = density.as<float>();
			if (auto& friction = boxCollider2DComponent["Friction"])
				bc2DC.Friction = friction.as<float>();
			if (auto& restitution = boxCollider2DComponent["Restitution"])
				bc2DC.Restitution = restitution.as<float>();
			if (auto& restitutionThreshold = boxCollider2DComponent["RestitutionThreshold"])
				bc2DC.RestitutionThreshold = restitutionThreshold.as<float>();
			if (auto& show = boxCollider2DComponent["Show"])
				bc2DC.Show = show.as<bool>();
		}

		// CircleCollider2DComponent
		if (auto& circleCollider2DComponent = eDetails["CircleCollider2DComponent"])
		{
			auto& cc2DC = scene->GetOrAddComponent<CircleCollider2DComponent>(entity);
			if(auto& offset = circleCollider2DComponent["Offset"])
				cc2DC.Offset = offset.as<glm::vec2>();
			if(auto& radius = circleCollider2DComponent["Radius"])
				cc2DC.Radius = radius.as<float>();
			if (auto& density = circleCollider2DComponent["Density"])
				cc2DC.Density = density.as<float>();
			if (auto& friction = circleCollider2DComponent["Friction"])
				cc2DC.Friction = friction.as<float>();
			if (auto& restitution = circleCollider2DComponent["Restitution"])
				cc2DC.Restitution = restitution.as<float>();
			if (auto& restitutionThreshold = circleCollider2DComponent["RestitutionThreshold"])
				cc2DC.RestitutionThreshold = restitutionThreshold.as<float>();
			if (auto& show = circleCollider2DComponent["Show"])
				cc2DC.Show = show.as<bool>();

		}
#pragma endregion

		return true;
	}

#pragma endregion

#pragma region Font

	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> func>
	static void LoadFontAtlas(const std::filesystem::path& filePath, Font::AtlasConfig& atlasConfig, Ref<Font::MSDFData> msdfData)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
		{
			GE_ERROR("Failed to load Font Freetype Handle");
			return;
		}

		std::filesystem::path path = Project::GetPathToAsset(filePath);
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, path.string().c_str());
		if (!font)
		{
			GE_ERROR("Failed to load Font Atlas.");
			return;
		}

		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		static const CharsetRange charsetRanges[] =
		{
			{ 0x0020, 0x00FF } // Basic Latin + Latin Supplement
		};

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t c = range.Begin; c <= range.End; c++)
				charset.add(c);
		}

		msdfData->FontGeometry = msdf_atlas::FontGeometry(&msdfData->Glyphs);
		int loadedGlyph = msdfData->FontGeometry.loadCharset(font, 1.0, charset);

		destroyFont(font);
		deinitializeFreetype(ft);

		float scale = 50.0;
		msdf_atlas::TightAtlasPacker atlasPacker;
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setSpacing(0);
		atlasPacker.setScale(scale);
		int remaining = atlasPacker.pack(msdfData->Glyphs.data(), (int)msdfData->Glyphs.size());

		int width, height;
		atlasPacker.getDimensions(width, height);
		scale = (float)atlasPacker.getScale();
		atlasConfig.Scale = scale;

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull

		if (atlasConfig.ExpensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = msdfData->Glyphs, &seed = atlasConfig.Seed](int i, int threadNum) -> bool
				{
					unsigned long long glyphSeed = (LCG_MULTIPLIER * (seed ^ i) + LCG_INCREMENT) * !!seed;
					glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
					return true;
				}, (int)msdfData->Glyphs.size()).finish(atlasConfig.ThreadCount);
		}
		else
		{
			unsigned long long glyphSeed = atlasConfig.Seed;
			for (msdf_atlas::GlyphGeometry& glyph : msdfData->Glyphs)
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		msdf_atlas::ImmediateAtlasGenerator<S, N, func, msdf_atlas::BitmapAtlasStorage<T, N>> atlasGenerator(width, height);
		{
			atlasGenerator.setAttributes(attributes);
			atlasGenerator.setThreadCount(8);
			atlasGenerator.generate(msdfData->Glyphs.data(), (int)msdfData->Glyphs.size());
		}

		msdfgen::BitmapConstRef<T, N> bitmap = atlasGenerator.atlasStorage();
		Texture::Config config = Texture::Config(bitmap.width, bitmap.height, 0, false);
		config.InternalFormat = Texture::ImageFormat::RGB8;
		config.Format = Texture::DataFormat::RGB;

		Buffer dataBuffer((void*)bitmap.pixels,
			bitmap.height * bitmap.width * (config.InternalFormat == Texture::ImageFormat::RGB8 ? 3 : 4));
		Ref<Texture2D> texture = Texture2D::Create(0, config, dataBuffer);
		dataBuffer.Release();

		atlasConfig.Texture = texture;
	}

#pragma endregion

#pragma region Texture2D

	/*
	* Expects full filePath
	*/
	static GE::Buffer LoadTextureDataFromFile(const std::string& filePath, int& width, int& height, int& channels)
	{
		GE_PROFILE_SCOPE("stbi_load - LoadTextureDataFromFile()");
		stbi_set_flip_vertically_on_load(1);
		// Size assumed 1 byte per channel
		uint8_t* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
		GE_CORE_ASSERT(data, "Failed to load stb_image!");
		GE::Buffer dataBuffer = GE::Buffer(data, width * height * channels);
		delete[] data;
		return dataBuffer;
	}

#pragma endregion

	AssetMetadata EditorAssetManager::s_NullMetadata = AssetMetadata();

	std::map<Pivot, std::string> EditorAssetManager::s_PivotStrs = 
	{
			{Pivot::Center, "Center"},
			{Pivot::LowerLeft, "LowerLeft"},
			{Pivot::TopLeft, "TopLeft"},
			{Pivot::TopRight, "TopRight"},
			{Pivot::LowerRight, "LowerRight"},
			{ Pivot::MiddleRight, "MiddleRight" },
			{ Pivot::TopMiddle, "TopMiddle" },
			{ Pivot::MiddleLeft, "MiddleLeft" },
			{ Pivot::BottomMiddle, "BottomMiddle" }
	};

	std::map<Layer::Type, std::string> EditorAssetManager::s_LayerTypeStrs =
	{
			{ Layer::Type::Debug, "Editor" },
			{ Layer::Type::Layer, "Game" },
			{ Layer::Type::GUI, "GUI" }
	};

	std::map<CanvasMode, std::string> EditorAssetManager::s_CanvasModeStrs =
	{
		{CanvasMode::None, "None"},
		{CanvasMode::Overlay, "Overlay"},
		{CanvasMode::World, "World"},
	};
	std::map<LayoutMode, std::string> EditorAssetManager::s_LayoutModeStrs =
	{
		{LayoutMode::Horizontal, "Horizontal"},
		{LayoutMode::Vertical, "Vertical"}
	};

	std::map<SliderDirection, std::string> EditorAssetManager::s_SliderDirectionStrs =
	{
		{SliderDirection::None, "None"},
		{SliderDirection::Left, "Left"},
		{SliderDirection::Center, "Center"},
		{SliderDirection::Right, "Right"},
		{SliderDirection::Top, "Top"},
		{SliderDirection::Middle, "Middle"},
		{SliderDirection::Bottom, "Bottom"}
	};

	EditorAssetManager::EditorAssetManager(const AssetMap& assetMap /*= AssetMap()*/) : m_LoadedAssets(assetMap)
	{
		m_AssetRegistry = CreateRef<AssetRegistry>();
	}

	EditorAssetManager::~EditorAssetManager()
	{
		InvalidateAssets();
		m_LoadedAssets.clear();
	}

	void EditorAssetManager::InvalidateAssets()
	{
		for (auto& [handle, asset] : m_LoadedAssets)
		{
			asset->Invalidate();
		}
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(UUID handle)
	{
		if (!HandleExists(handle))
		{
			GE_WARN("Asset Handle doesn't exist in EditorAssetManager. \n\tReturning null Metadata.");
			return s_NullMetadata;
		}
		return m_AssetRegistry->GetAssetMetadata(handle);
	}

	Ref<Asset> EditorAssetManager::GetAsset(UUID handle)
	{
		Ref<Asset> asset = nullptr;
		if (HandleExists(handle))
		{
			if (AssetLoaded(handle))
				asset = m_LoadedAssets.at(handle);
			else
				asset = DeserializeAsset(m_AssetRegistry->GetAssetMetadata(handle));
		}
		return asset;
	}

	const AssetMap& EditorAssetManager::GetLoadedAssets()
	{
		return m_LoadedAssets;
	}

	Ref<Asset> EditorAssetManager::GetAsset(const std::filesystem::path& filePath)
	{
		UUID assetHandle = 0;
		const auto& registry = m_AssetRegistry->GetRegistry();
		for (const auto& [handle, metadata] : registry)
		{
			if (metadata.FilePath == filePath)
				assetHandle = handle;
		}

		return GetAsset(assetHandle);
	}

	bool EditorAssetManager::HandleExists(UUID handle)
	{
		return m_AssetRegistry->AssetExists(handle);
	}

	bool EditorAssetManager::AssetLoaded(UUID handle)
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	bool EditorAssetManager::AddAsset(Ref<Asset> asset)
	{
		if (!asset)
			return false;
		UUID handle = asset->GetHandle();
		// Special case for Scripts
		if (asset->GetType() == Asset::Type::Script && !HandleExists(handle))
		{
			AssetMetadata metadata = AssetMetadata();
			Ref<Script> script = Project::GetAssetAs<Script>(asset);
			if (script)
			{
				std::filesystem::path className = script->GetName() + ".cs";
				metadata = AssetMetadata(asset->GetHandle(), className);
				AddAsset(metadata);
			}
		}

		if (AssetLoaded(handle))
		{
			GE_WARN("Cannot add Asset to Loaded. Asset already in Loaded.");
			return false;
		}
		m_LoadedAssets.emplace(handle, asset);
		return true;
	}

	bool EditorAssetManager::AddAsset(const AssetMetadata& metadata)
	{
		return m_AssetRegistry->AddAsset(metadata);
	}

	bool EditorAssetManager::RemoveAsset(UUID handle)
	{
		if (!HandleExists(handle))
			return false;

		if (AssetLoaded(handle))
			m_LoadedAssets.erase(handle);

		return m_AssetRegistry->RemoveAsset(handle);
	}

	bool EditorAssetManager::SerializeAsset(Ref<Asset> asset, const AssetMetadata& metadata)
	{
		switch (asset->GetType())
		{
		case Asset::Type::Scene:
			return SerializeScene(asset, metadata);
			break;
		default:
			GE_ERROR("Serialize metadata function not found for Type: " + AssetUtils::AssetTypeToString(metadata.Type));
			break;
		}
		return false;
	}

	Ref<Asset> EditorAssetManager::DeserializeAsset(const AssetMetadata& metadata)
	{
		if (AssetLoaded(metadata.Handle))
			return m_LoadedAssets.at(metadata.Handle);

		Ref<Asset> asset = nullptr;
		switch (metadata.Type)
		{
		case Asset::Type::Scene:
			asset = DeserializeScene(metadata);
			break;
		case Asset::Type::Texture2D:
			asset = DeserializeTexture2D(metadata);
			break;
		case Asset::Type::Font:
			asset = DeserializeFont(metadata);
			break;
		case Asset::Type::Audio:
			asset = DeserializeAudio(metadata);
			break;
		case Asset::Type::Script:
			asset = DeserializeScript(metadata);
			break;
		default:
			GE_ERROR("Deserialize metadata function not found for Type: {0}", AssetUtils::AssetTypeToString(metadata.Type));
			return nullptr;
			break;
		}

		AddAsset(asset);

		return asset;
	}

	bool EditorAssetManager::SerializeAssets()
	{
		// Prepare assets
		for (auto& [className, script] : Scripting::GetScripts())
		{
			UUID scriptHandle = script->GetHandle();
			if (!m_AssetRegistry->AssetExists(scriptHandle))
			{
				std::filesystem::path filePath = std::filesystem::path(script->GetName() + ".cs");
				AssetMetadata metadata = AssetMetadata(scriptHandle, filePath);
				m_AssetRegistry->AddAsset(metadata);
			}
		}
		for (const auto& [handle, metadata] : m_AssetRegistry->GetRegistry())
		{
			// if Asset isn't Scene, get from loaded. Otherwise, get runtime Scene from Project.
			// This ensures that changes made to the Scene are saved.
			Ref<Asset> asset = ((AssetLoaded(handle) && m_LoadedAssets.at(handle)->GetType() != Asset::Type::Scene) 
				? m_LoadedAssets.at(handle) : Project::GetRuntimeScene());

			SerializeAsset(asset, metadata);
		}

		// Serialize into .gar(Game Asset Registry) file
		std::filesystem::path path = Project::GetPathToAsset(m_AssetRegistry->GetFilePath());
		GE_INFO("AssetRegistry Serialization Started.\n\tFilePath : {0}", path.string().c_str());

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;
			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : m_AssetRegistry->GetRegistry())
			{
				GE_TRACE("UUID : {0}\n\tFilePath : {1}\n\tType : {2}", (uint64_t)handle, metadata.FilePath.string().c_str(), AssetUtils::AssetTypeToString(metadata.Type).c_str());

				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				std::string filepathStr = metadata.FilePath.generic_string();
				out << YAML::Key << "FilePath" << YAML::Value << filepathStr;
				const std::string typeString = AssetUtils::AssetTypeToString(metadata.Type);
				out << YAML::Key << "Type" << YAML::Value << typeString.c_str();
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(path);
		if (fout.is_open() && fout.good())
		{
			fout << out.c_str();
			fout.close();
			GE_INFO("Asset Registry Serialization Complete.");
			return true;
		}

		GE_WARN("Asset Registry Serialization Failed.");
		return false;
	}

	bool EditorAssetManager::DeserializeAssets()
	{
		m_LoadedAssets.clear();
		m_LoadedAssets = AssetMap();
		m_AssetRegistry->Clear();
		const std::filesystem::path& path = Project::GetPathToAsset(m_AssetRegistry->GetFilePath());
		if (path.empty())
			return false;
		GE_TRACE("Asset Registry Deserialization Started.\n\tFilePath : {0}", path.string().c_str());

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
			GE_ERROR("Failed to load Asset Registry file. {0}\n\t{1}", path.string().c_str(), e.what());
			return false;
		}

		YAML::Node assetRegistryData = data["AssetRegistry"];
		if (!assetRegistryData)
		{
			GE_WARN("Asset Registry Deserialization Failed.\n\tAssetRegistry Node not found.");
			return false;
		}

		for (const auto& node : assetRegistryData)
		{
			AssetMetadata assetMetadata;
			assetMetadata.Handle = node["Handle"].as<uint64_t>();
			assetMetadata.FilePath = node["FilePath"].as<std::string>();
			assetMetadata.Type = AssetUtils::AssetTypeFromString(node["Type"].as<std::string>());
			GE_TRACE("UUID : {0}\n\tFilePath : {1}\n\tType : {2}", (uint64_t)assetMetadata.Handle, assetMetadata.FilePath.string().c_str(), AssetUtils::AssetTypeToString(assetMetadata.Type).c_str());
			if (!m_AssetRegistry->AddAsset(assetMetadata))
				GE_WARN("Failed to add Asset::{0} : {1}\n\tFilePath : {2}", AssetUtils::AssetTypeToString(assetMetadata.Type).c_str(), (uint64_t)assetMetadata.Handle, assetMetadata.FilePath.string().c_str());
		}

		for (const auto& [handle, metadata] : m_AssetRegistry->GetRegistry())
		{
			DeserializeAsset(metadata);
		}
		GE_INFO("Asset Registry Deserialization Complete.");
		return true;
	}

#pragma region Asset Metadata Serialization

	Ref<Asset> EditorAssetManager::DeserializeScene(const AssetMetadata& metadata)
	{
		std::filesystem::path path = Project::GetPathToAsset(metadata.FilePath);

		std::ifstream stream(path);
		std::stringstream strStream;
		if (stream.good())
		{
			strStream << stream.rdbuf();
		}
		YAML::Node data;
		try
		{
			data = YAML::Load(strStream.str());
		}
		catch (YAML::ParserException e)
		{
			GE_ERROR("Failed to load Scene Asset file. {0}\n\t{1}", path.string(), e.what());
		}

		Ref<Scene> scene = nullptr;
		if (auto& sceneName = data["Scene"])
		{
			std::string name = sceneName.as<std::string>();
			scene = CreateRef<Scene>(metadata.Handle);
			GE_TRACE("Deserializing Scene\n\tUUID : {0}\n\tName : {1}\n\tPath : {2}", (uint64_t)metadata.Handle, name.c_str(), path.string().c_str());

			if (auto& entities = data["Entities"])
			{
				GE_TRACE("Deserializing Entities");

				for (auto& entityDetails : entities)
				{
					// ID Component 
					UUID uuid = 0;
					if(auto& entityUUID = entityDetails["Entity"])
						uuid = entityUUID.as<UUID>();

					// TagComponent
					std::string tagStr = std::string();
					uint32_t tcID = 0;
					if(auto& tc = entityDetails["TagComponent"])
					{
						if(auto& tag = tc["Tag"])
							tagStr = tag.as<std::string>();
						if(auto& tagID = tc["ID"])
							tcID = tagID.as<uint32_t>();
						if (!tagStr.empty())
							Project::AddTag(tagStr, tcID);
					}

					// NameComponent
					if (auto& nc = entityDetails["NameComponent"])
					{
						std::string name = std::string("New Entity");
						if(auto& entityName = nc["Name"])
							name = entityName.as<std::string>();

						Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name, tcID);
						if (DeserializeEntity(scene, entityDetails, deserializedEntity))
							GE_TRACE("UUID : {0},\n\tTag : {1}\n\tName : {2}", (uint64_t)uuid, Project::GetStrByTag(scene->GetComponent<TagComponent>(deserializedEntity).TagID).c_str(), name.c_str());
						else
							GE_ERROR("Failed to Deserialize Entity.");
					}
				}
			}

		}
		return scene;
	}

	Ref<Asset> EditorAssetManager::DeserializeTexture2D(const AssetMetadata& metadata)
	{
		int width = 0, height = 0, channels = 4;
		stbi_set_flip_vertically_on_load(1);
		Buffer data = LoadTextureDataFromFile(Project::GetPathToAsset(metadata.FilePath).string(), width, height, channels);

		Texture::Config config = Texture::Config(width, height, channels, true);
		Ref<Texture2D> texture = Texture2D::Create(metadata.Handle, config, data);
		data.Release();

		return texture;
	}

	Ref<Asset> EditorAssetManager::DeserializeFont(const AssetMetadata& metadata)
	{
		Font::AtlasConfig config = Font::AtlasConfig();
		Ref<Font::MSDFData> data = CreateRef<Font::MSDFData>();
		LoadFontAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(metadata.FilePath, config, data);
		Ref<Font> asset = CreateRef<Font>(metadata.Handle, config, data);
		return asset;
	}

	Ref<Asset> EditorAssetManager::DeserializeAudio(const AssetMetadata& metadata)
	{
		// Read data from filePath
		{
			const std::filesystem::path filePath = Project::GetPathToAsset(metadata.FilePath);
			std::ifstream stream(filePath, std::ios::binary);
			if (!stream.is_open())
			{
				GE_ERROR("Could not open WAV file {0}", filePath.string().c_str());
				return nullptr;
			}

			{
				char data[4];

				// the RIFF
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read RIFF while loading Wav file.");
					return false;
				}
				if (std::strncmp(data, "RIFF", 4) != 0)
				{
					GE_ERROR("File is not a valid WAVE file (header doesn't begin with RIFF)");
					return false;
				}

				// the size of the file
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read size of Wav file.");
					return false;
				}

				// the WAVE
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read WAVE");
					return false;
				}
				if (std::strncmp(data, "WAVE", 4) != 0)
				{
					GE_ERROR("File is not a valid WAVE file (header doesn't contain WAVE)");
					return false;
				}

				// "fmt/0"
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read fmt of Wav file.");
					return false;
				}

				// this is always 16, the size of the fmt data chunk
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read the size of the fmt data chunk. Should be 16.");
					return false;
				}

				// PCM should be 1?
				if (!stream.read(data, 2))
				{
					GE_ERROR("Could not read PCM. Should be 1.");
					return false;
				}

				// the number of Channels
				if (!stream.read(data, 2))
				{
					GE_ERROR("Could not read number of Channels.");
					return false;
				}
				uint32_t channels = convert<uint32_t>(data, 2);

				// sample rate
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read sample rate.");
					return false;
				}
				uint32_t sampleRate = convert<uint32_t>(data, 4);

				// (SampleRate * BPS * Channels) / 8
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read (SampleRate * BPS * Channels) / 8");
					return false;
				}

				// ?? dafaq
				if (!stream.read(data, 2))
				{
					GE_ERROR("Could not read dafaq?");
					return false;
				}

				// BPS
				if (!stream.read(data, 2))
				{
					GE_ERROR("Could not read bits per sample.");
					return false;
				}
				uint32_t bps = convert<uint32_t>(data, 2);

				// data chunk header "data"
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read data chunk header.");
					return false;
				}
				if (std::strncmp(data, "data", 4) != 0)
				{
					GE_ERROR("File is not a valid WAVE file (doesn't have 'data' tag).");
					return false;
				}

				// size of data
				if (!stream.read(data, 4))
				{
					GE_ERROR("Could not read data size.");
					return false;
				}
				size_t size = convert<uint32_t>(data, 4);

				/* cannot be at the end of file */
				if (stream.eof())
				{
					GE_ERROR("Reached EOF.");
					return false;
				}

				uint64_t bufferCount = size > AudioManager::BUFFER_SIZE ? (size / AudioManager::BUFFER_SIZE) + 1 : 1;
				Buffer buffer = Buffer(AudioManager::BUFFER_SIZE * bufferCount);
				// Read Data
				if (!stream.read(buffer.As<char>(), size))
				{
					GE_ERROR("Could not read data.");
					return false;
				}

				if (stream.fail())
				{
					GE_ERROR("File stream Failed.");
					return false;
				}
				stream.close();

				Ref<Audio> audio = Audio::Create(metadata.Handle, Audio::Config(channels, sampleRate, bps, buffer), (uint32_t)bufferCount);
				buffer.Release();

				return audio;
			}

		}
		return nullptr;
	}

	Ref<Asset> EditorAssetManager::DeserializeScript(const AssetMetadata& metadata)
	{
		std::filesystem::path fileName = metadata.FilePath.filename();
		std::string className = fileName.stem().string();
		const std::unordered_map<std::string, Ref<Script>>& scripts = Scripting::GetScripts();
		if (scripts.find(className) != scripts.end())
		{
			Scripting::SetScriptHandle(className, metadata.Handle);
			return scripts.at(className);
		}

		return nullptr;
	}

	bool EditorAssetManager::SerializeScene(Ref<Asset> asset, const AssetMetadata& metadata)
	{
		std::filesystem::path path = Project::GetPathToAsset(metadata.FilePath);
		std::string sceneName = metadata.FilePath.filename().string();
		Ref<Scene> scene = Project::GetAssetAs<Scene>(asset);
		GE_TRACE("Serializing Scene\n\tUUID : {0}\n\tName : {1}\n\tPath : {2}", (uint64_t)metadata.Handle, sceneName.c_str(), path.string());

		YAML::Emitter out;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Scene" << YAML::Value << sceneName;
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

			std::vector<Entity> entities = scene->GetAllEntitiesWith<IDComponent>();
			for (Entity entity : entities)
			{
				SerializeEntity(out, scene, entity);
			}
			entities.clear();
			entities = std::vector<Entity>();

			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		std::ofstream fout(path);
		if (fout.is_open() && fout.good())
		{
			fout << out.c_str();
			fout.close();
			GE_TRACE("Scene Serialization Complete.");
			return true;
		}

		GE_WARN("Scene Serialization Failed.");
		return false;
	}

#pragma endregion
}
