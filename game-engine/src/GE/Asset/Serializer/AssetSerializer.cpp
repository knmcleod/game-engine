#include "GE/GEpch.h"

#include "AssetSerializer.h"

#include "GE/Asset/Assets/Audio/Audio.h"
#include "GE/Asset/Assets/Scene/Scene.h"

#include "GE/Asset/RuntimeAssetManager.h"

#include "GE/Project/Project.h"

#include "GE/Scripting/Scripting.h"

#include <stb_image.h>
#include <yaml-cpp/yaml.h>
#include <chrono>

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
	std::map<Asset::Type, AssetDeserializeFunction> AssetSerializer::s_AssetDeserializeFuncs =
	{
		{ Asset::Type::Scene,		AssetSerializer::DeserializeScene		},
		{ Asset::Type::Texture2D,	AssetSerializer::DeserializeTexture2D	},
		{ Asset::Type::Font,		AssetSerializer::DeserializeFont		},
		{ Asset::Type::AudioClip,	AssetSerializer::DeserializeAudio		},
	};

	std::map<Asset::Type, AssetPackDeserializeFunction> AssetSerializer::s_AssetPackDeserializeFuncs =
	{
		{ Asset::Type::Scene,		AssetSerializer::DeserializeSceneFromPack		},
		{ Asset::Type::Texture2D,	AssetSerializer::DeserializeTexture2DFromPack	},
		{ Asset::Type::Font,		AssetSerializer::DeserializeFontFromPack		},
		{ Asset::Type::AudioClip,	AssetSerializer::DeserializeAudioFromPack		},
	};

	std::map<Asset::Type, AssetSerializeFunction> AssetSerializer::s_AssetSerializeFuncs =
	{
		{ Asset::Type::Scene, AssetSerializer::SerializeScene }
	};

	std::map<Asset::Type, AssetPackSerializeFunction> AssetSerializer::s_AssetPackSerializeFuncs =
	{
		{ Asset::Type::Scene, AssetSerializer::SerializeSceneForPack },
		{ Asset::Type::Texture2D, AssetSerializer::SerializeTexture2DForPack },
		{ Asset::Type::Font, AssetSerializer::SerializeFontForPack },
		{ Asset::Type::AudioClip, AssetSerializer::SerializeAudioForPack }
	};

#pragma region Entity

	static void SerializeEntity(YAML::Emitter& out, const Entity& entity)
	{
		GE_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Cannot serialize Entity without ID.");

		UUID entityHandle = entity.GetComponent<IDComponent>().ID;
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entityHandle;

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			GE_CORE_TRACE("Serializing entity\n\tUUID: {0}\n\tName: {1}", (uint64_t)entityHandle, tag.c_str());
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent
			auto& component = entity.GetComponent<TransformComponent>();

			out << YAML::Key << "Translation" << YAML::Value << component.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << component.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << component.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent
			auto& component = entity.GetComponent<CameraComponent>();
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

		if (entity.HasComponent<AudioSourceComponent>())
		{
			out << YAML::Key << "AudioSourceComponent";
			out << YAML::BeginMap; // AudioSourceComponent
			auto& component = entity.GetComponent<AudioSourceComponent>();

			out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;
			out << YAML::Key << "Gain" << YAML::Value << component.Gain;
			out << YAML::Key << "Pitch" << YAML::Value << component.Pitch;
			out << YAML::Key << "Loop" << YAML::Value << component.Loop;

			out << YAML::EndMap; // AudioSourceComponent
		}

		if (entity.HasComponent<AudioListenerComponent>())
		{
			out << YAML::Key << "AudioListenerComponent";
			out << YAML::BeginMap; // AudioListenerComponent
			auto& component = entity.GetComponent<AudioListenerComponent>();

			out << YAML::EndMap; // AudioListenerComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent
			auto& component = entity.GetComponent<SpriteRendererComponent>();
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

		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent
			auto& component = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << component.Color;
			out << YAML::Key << "Radius" << YAML::Value << component.Radius;
			out << YAML::Key << "Thickness" << YAML::Value << component.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << component.Fade;
			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.HasComponent<TextRendererComponent>())
		{
			out << YAML::Key << "TextRendererComponent";
			out << YAML::BeginMap; // TextRendererComponent
			auto& component = entity.GetComponent<TextRendererComponent>();
			out << YAML::Key << "TextColor" << YAML::Value << component.TextColor;
			out << YAML::Key << "BGColor" << YAML::Value << component.BGColor;

			out << YAML::Key << "KerningOffset" << YAML::Value << component.KerningOffset;
			out << YAML::Key << "LineHeightOffset" << YAML::Value << component.LineHeightOffset;
			out << YAML::Key << "Text" << YAML::Value << component.Text;

			out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;
			out << YAML::EndMap; // TextRendererComponent
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap; // NativeScriptComponent
			auto& component = entity.GetComponent<NativeScriptComponent>();

			out << YAML::EndMap; // NativeScriptComponent
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent
			auto& component = entity.GetComponent<ScriptComponent>();
			out << YAML::Key << "ClassName" << YAML::Value << component.ClassName;

			// Fields
			Ref<ScriptClass> scriptClass = Scripting::GetScriptClass(component.ClassName);
			if (scriptClass)
			{
				const auto& fields = scriptClass->GetFields();
				if (fields.size() > 0)
				{
					out << YAML::Key << "ScriptFields" << YAML::Value;
					out << YAML::BeginSeq;

					auto& scriptFieldMap = Scripting::GetScriptFieldMap(entity);
					for (const auto& [name, field] : fields)
					{
						if (scriptFieldMap.find(name) == scriptFieldMap.end())
							continue;
						ScriptField::Type type = field.GetType();
						out << YAML::BeginMap; // ScriptFields

						out << YAML::Key << "Name" << YAML::Value << name;
						out << YAML::Key << "Type" << YAML::Value << Scripting::ScriptFieldTypeToString(type);
						out << YAML::Key << "Data" << YAML::Value;

						ScriptField& scriptField = scriptFieldMap.at(name);
						switch (type)
						{
						case ScriptField::Type::Char:
							out << scriptField.GetValue<char>();
							break;
						case ScriptField::Type::Int:
							out << scriptField.GetValue<int>();
							break;
						case ScriptField::Type::UInt:
							out << scriptField.GetValue<uint32_t>();
							break;
						case ScriptField::Type::Float:
							out << scriptField.GetValue<float>();
							break;
						case ScriptField::Type::Byte:
							out << scriptField.GetValue<int8_t>();
							break;
						case ScriptField::Type::Bool:
							out << scriptField.GetValue<bool>();
							break;
						case ScriptField::Type::Vector2:
							out << scriptField.GetValue<glm::vec2>();
							break;
						case ScriptField::Type::Vector3:
							out << scriptField.GetValue<glm::vec3>();
							break;
						case ScriptField::Type::Vector4:
							out << scriptField.GetValue<glm::vec4>();
							break;
						case ScriptField::Type::Entity:
							out << scriptField.GetValue<UUID>();
							break;
						}

						out << YAML::EndMap; // ScriptFields
					}
					out << YAML::EndSeq;
				}
			}
			out << YAML::EndMap; // ScriptComponent
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent
			auto& component = entity.GetComponent<Rigidbody2DComponent>();
			const std::string typeString = ComponentUtils::GetStringFromRigidBody2DType(component.Type);
			out << YAML::Key << "Type" << YAML::Value << typeString;
			out << YAML::Key << "FixedRotation" << YAML::Value << component.FixedRotation;
			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent
			auto& component = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "Size" << YAML::Value << component.Size;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;
			out << YAML::Key << "Show" << YAML::Value << component.Show;
			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent
			auto& component = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "Radius" << YAML::Value << component.Radius;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;
			out << YAML::Key << "Show" << YAML::Value << component.Show;
			out << YAML::EndMap; // CircleCollider2DComponent
		}

		out << YAML::EndMap; // Entity
	}

	static bool DeserializeEntity(const YAML::detail::iterator_value& eDetails, Entity& entity)
	{
		// TransformComponent
		auto transformComponent = eDetails["TransformComponent"];
		if (transformComponent)
		{
			auto& tc = entity.GetOrAddComponent<TransformComponent>();
			tc.Translation = transformComponent["Translation"].as<glm::vec3>();
			tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
			tc.Scale = transformComponent["Scale"].as<glm::vec3>();
		}

		// CameraComponent
		auto cameraComponent = eDetails["CameraComponent"];
		if (cameraComponent)
		{
			auto& cc = entity.GetOrAddComponent<CameraComponent>();
			auto& cameraProps = cameraComponent["Camera"];

			cc.ActiveCamera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["Type"].as<int>());
			cc.ActiveCamera.SetFOV(cameraProps["FOV"].as<float>());
			cc.ActiveCamera.SetNearClip(cameraProps["Near"].as<float>());
			cc.ActiveCamera.SetFarClip(cameraProps["Far"].as<float>());

			cc.Primary = cameraComponent["Primary"].as<bool>();
			cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
		}

		// AudioSourceComponent
		auto audioSourceComponent = eDetails["AudioSourceComponent"];
		if (audioSourceComponent)
		{
			auto& cc = entity.GetOrAddComponent<AudioSourceComponent>();
			cc.AssetHandle = audioSourceComponent["AssetHandle"].as<UUID>();
			cc.Gain = audioSourceComponent["Gain"].as<float>();
			cc.Pitch = audioSourceComponent["Pitch"].as<float>();
			cc.Loop = audioSourceComponent["Loop"].as<bool>();
		}

		// AudioListenerComponent
		auto audioListenerComponent = eDetails["AudioListenerComponent"];
		if (audioListenerComponent)
		{
			auto& cc = entity.GetOrAddComponent<AudioListenerComponent>();

		}

		// SpriteRendererComponent
		auto spriteRendererComponent = eDetails["SpriteRendererComponent"];
		if (spriteRendererComponent)
		{
			auto& src = entity.GetOrAddComponent<SpriteRendererComponent>();
			src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
			if (spriteRendererComponent["AssetHandle"])
			{
				src.AssetHandle = spriteRendererComponent["AssetHandle"].as<UUID>();
			}

			if (spriteRendererComponent["TilingFactor"])
			{
				src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
			}
		}

		// CircleRendererComponent
		auto circleRendererComponent = eDetails["CircleRendererComponent"];
		if (circleRendererComponent)
		{
			auto& src = entity.GetOrAddComponent<CircleRendererComponent>();
			src.Color = circleRendererComponent["Color"].as<glm::vec4>();
			src.Radius = circleRendererComponent["Radius"].as<float>();
			src.Thickness = circleRendererComponent["Thickness"].as<float>();
			src.Fade = circleRendererComponent["Fade"].as<float>();
		}

		// TextRendererComponent
		auto textRendererComponent = eDetails["TextRendererComponent"];
		if (textRendererComponent)
		{
			auto& src = entity.GetOrAddComponent<TextRendererComponent>();
			src.TextColor = textRendererComponent["TextColor"].as<glm::vec4>();
			src.BGColor = textRendererComponent["BGColor"].as<glm::vec4>();

			src.KerningOffset = textRendererComponent["KerningOffset"].as<float>();
			src.LineHeightOffset = textRendererComponent["LineHeightOffset"].as<float>();
			src.Text = textRendererComponent["Text"].as<std::string>();

			src.AssetHandle = textRendererComponent["AssetHandle"].as<UUID>();
		}

		// NativeScriptComponent
		auto nativeScriptComponent = eDetails["NativeScriptComponent"];
		if (nativeScriptComponent)
		{
			auto& src = entity.GetOrAddComponent<NativeScriptComponent>();
		}

		// ScriptComponent
		auto scriptComponent = eDetails["ScriptComponent"];
		if (scriptComponent)
		{
			Scripting::SetScene(entity.GetScene());

			auto& src = entity.GetOrAddComponent<ScriptComponent>();
			src.ClassName = scriptComponent["ClassName"].as<std::string>();

			// Fields
			auto fields = scriptComponent["ScriptFields"];
			if (fields)
			{
				Ref<ScriptClass> scriptClass = Scripting::GetScriptClass(src.ClassName);
				GE_CORE_ASSERT(scriptClass, "Script Class does not exist.");
				const auto& scriptClassFields = scriptClass->GetFields();
				auto& scriptFieldMap = Scripting::GetScriptFieldMap(entity);

				for (auto field : fields)
				{
					std::string scriptFieldName = field["Name"].as<std::string>();

					if (scriptClassFields.find(scriptFieldName) == scriptClassFields.end())
					{
						GE_CORE_WARN("Cannot deserialize Entity Script Field.\n\tScript Field Name: {0} not found.", scriptFieldName);
						continue;
					}

					std::string scriptFieldTypeString = field["Type"].as<std::string>();
					ScriptField::Type scriptFieldType = Scripting::StringToScriptFieldType(scriptFieldTypeString);

					ScriptField& scriptField = scriptFieldMap[scriptFieldName];

					switch (scriptFieldType)
					{
					case GE::ScriptField::Type::None:
						break;
					case GE::ScriptField::Type::Char:
					{
						char data = field["Data"].as<char>();
						scriptField.SetValue<char>(data);
						break;
					}
					case GE::ScriptField::Type::Int:
					{
						int32_t data = field["Data"].as<int32_t>();
						scriptField.SetValue<int32_t>(data);
						break;
					}
					case GE::ScriptField::Type::UInt:
					{
						uint32_t data = field["Data"].as<uint32_t>();
						scriptField.SetValue<uint32_t>(data);
						break;
					}
					case GE::ScriptField::Type::Float:
					{
						float data = field["Data"].as<float>();
						scriptField.SetValue<float>(data);
						break;
					}
					case GE::ScriptField::Type::Byte:
					{
						int8_t data = field["Data"].as<int8_t>();
						scriptField.SetValue<int8_t>(data);
						break;
					}
					case GE::ScriptField::Type::Bool:
					{
						bool data = field["Data"].as<bool>();
						scriptField.SetValue<bool>(data);
						break;
					}
					case GE::ScriptField::Type::Vector2:
					{
						glm::vec2 data = field["Data"].as<glm::vec2>();
						scriptField.SetValue<glm::vec2>(data);
						break;
					}
					case GE::ScriptField::Type::Vector3:
					{
						glm::vec3 data = field["Data"].as<glm::vec3>();
						scriptField.SetValue<glm::vec3>(data);
						break;
					}
					case GE::ScriptField::Type::Vector4:
					{
						glm::vec4 data = field["Data"].as<glm::vec4>();
						scriptField.SetValue<glm::vec4>(data);
						break;
					}
					case GE::ScriptField::Type::Entity:
					{
						UUID data = field["Data"].as<UUID>();
						scriptField.SetValue<UUID>(data);
						break;
					}
					}

				}
			}

		}

		// RigidBody2DComponent
		auto rigidBody2DComponent = eDetails["Rigidbody2DComponent"];
		if (rigidBody2DComponent)
		{
			auto& src = entity.GetOrAddComponent<Rigidbody2DComponent>();
			src.FixedRotation = rigidBody2DComponent["FixedRotation"].as<bool>();
			src.Type = ComponentUtils::GetRigidBody2DTypeFromString(rigidBody2DComponent["Type"].as<std::string>());
		}

		// BoxCollider2DComponent
		auto boxCollider2DComponent = eDetails["BoxCollider2DComponent"];
		if (boxCollider2DComponent)
		{
			auto& src = entity.GetOrAddComponent<BoxCollider2DComponent>();
			src.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
			src.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
			src.Density = boxCollider2DComponent["Density"].as<float>();
			src.Friction = boxCollider2DComponent["Friction"].as<float>();
			src.Restitution = boxCollider2DComponent["Restitution"].as<float>();
			src.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();

			src.Show = boxCollider2DComponent["Show"].as<bool>();
		}

		// CircleCollider2DComponent
		auto circleCollider2DComponent = eDetails["CircleCollider2DComponent"];
		if (circleCollider2DComponent)
		{
			auto& src = entity.GetOrAddComponent<CircleCollider2DComponent>();
			src.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
			src.Radius = circleCollider2DComponent["Radius"].as<float>();
			src.Density = circleCollider2DComponent["Density"].as<float>();
			src.Friction = circleCollider2DComponent["Friction"].as<float>();
			src.Restitution = circleCollider2DComponent["Restitution"].as<float>();
			src.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();

			src.Show = circleCollider2DComponent["Show"].as<bool>();
		}

		return true;
	}

	static bool SerializeEntity(SceneInfo::EntityInfo& entityInfo, const Entity& entity)
	{
		/*
		* Contains
		* - Offset : Offset of UUID relative to Parent Scene
		* - Size : Packed Data Size
		* ~ Data : Packed Data
		* * ~ Components : Components on Entity
		* * * - ID : UUID
		* * * - Tag : Name
		* * * ~ Transform :
		* * * * - Translation :
		* * * * - Rotation :
		* * * * - Scale :
		* * * ~ Camera :
		* * * * ~ SceneCamera :
		* * * * * - Fov : float
		* * * * * - NearClip : float
		* * * * * - FarClip : float
		* * * * - Primary : bool
		* * * * - FixedAspectRatio : bool
		* * * ~ AudioSource :
		* * * * - Asset Handle :
		* * * * - Loop : bool
		* * * * - Pitch : float
		* * * * - Gain : float
		* * * ~ AudioListener :
		* * * * -
		* * * ~ SpriteRenderer :
		* * * * - Asset Handle :
		* * * * - Color : vec4
		* * * * - TilingFactor : float
		* * * ~ CircleRenderer :
		* * * * - Asset Handle :
		* * * * - Color : vec4
		* * * * - TilingFactor : float
		* * * * - Radius : float
		* * * * - Thickness : float
		* * * * - Fade : float
		* * * ~ TextRenderer :
		* * * * - Asset Handle :
		* * * * - TextColor : vec4
		* * * * - BGColor : vec4
		* * * * - KerningOffset : float
		* * * * - LineHeightOffset : float
		* * * * - Text : std::string
		* * * ~ Rigidbody2D :
		* * * * - Type : uint16_t
		* * * * - FixedRotation : bool
		* * * ~ BoxCollider2D :
		* * * * - Show : bool
		* * * * - Density : float
		* * * * - Friction : float
		* * * * - Restitution : float
		* * * * - RestitutionThreshold : float
		* * * * - Offset : vec2
		* * * * - Size : vec2
		* * * ~ CircleCollider2D :
		* * * * - Show : bool
		* * * * - Radius : float
		* * * * - Density : float
		* * * * - Friction : float
		* * * * - Restitution : float
		* * * * - RestitutionThreshold : float
		* * * * - Offset : vec2
		* * * ~ NativeScript :
		* * * * -
		* * * ~ Script :
		* * * * - ClassName : std::string
		* * * * ~ Fields :
		* * * * * - Name : std::string
		* * * * * - Type : char*
		* * * * * - Data :
		*/
		
		uint64_t requiredSize = 0;

		// Get Size
		{
			// ID Component
			requiredSize += Aligned(sizeof(uint64_t)); // UUID

			// Tag Component 
			if (entity.HasComponent<TagComponent>())
			{
				TagComponent tc = entity.GetComponent<TagComponent>();
				requiredSize += Aligned(sizeof(uint64_t)); // Name Length
				requiredSize += Aligned(tc.Tag.size() * sizeof(char)); // Name String
			}

			// Transform
			if (entity.HasComponent<TransformComponent>())
			{
				// Translation
				requiredSize += Aligned(sizeof(float))		// x position : float
					+ Aligned(sizeof(float))				// y position : float
					+ Aligned(sizeof(float));				// z position : float

				// Rotation
				requiredSize += Aligned(sizeof(float))		// x rotation : float
					+ Aligned(sizeof(float))				// y rotation : float
					+ Aligned(sizeof(float));				// z rotation : float

				// Scale
				requiredSize += Aligned(sizeof(float))		// x scale : float
					+ Aligned(sizeof(float))				// y scale : float
					+ Aligned(sizeof(float));				// z scale : float
			}

			// Camera
			if (entity.HasComponent<CameraComponent>())
			{
				requiredSize += Aligned(sizeof(bool))	// Primary
					+ Aligned(sizeof(bool));			// FixedAspectRatio

				// SceneCamera Variables
				requiredSize += Aligned(sizeof(float))	// FOV
					+ Aligned(sizeof(float))			// NearCip
					+ Aligned(sizeof(float));			// FarCip
			}

			// AudioSource
			if (entity.HasComponent<AudioSourceComponent>())
			{
				requiredSize += Aligned(sizeof(uint64_t))	// Audio Asset UUID, See Assets(UUID)
					+ Aligned(sizeof(bool))					// Loop
					+ Aligned(sizeof(float))				// Pitch
					+ Aligned(sizeof(float));				//Gain

			}

			// AudioListener
			if (entity.HasComponent<AudioListenerComponent>())
			{

			}

			// SpriteRenderer
			if (entity.HasComponent<SpriteRendererComponent>())
			{
				SpriteRendererComponent src = entity.GetComponent<SpriteRendererComponent>();

				requiredSize += Aligned(sizeof(uint64_t))	// Texture Asset UUID
					+ Aligned(sizeof(float));				// TilingFactor

				// Color
				requiredSize += Aligned(sizeof(float))	// r
					+ Aligned(sizeof(float))			// g
					+ Aligned(sizeof(float))			// b
					+ Aligned(sizeof(float));			// a
			}

			// CircleRenderer
			if (entity.HasComponent<CircleRendererComponent>())
			{
				CircleRendererComponent crc = entity.GetComponent<CircleRendererComponent>();

				requiredSize += Aligned(sizeof(uint64_t))	// Texture Asset UUID
					+ Aligned(sizeof(float))				// TilingFactor
					+ Aligned(sizeof(float))				// Radius
					+ Aligned(sizeof(float))				// Thickness
					+ Aligned(sizeof(float));				// Fade

				// Color
				requiredSize += Aligned(sizeof(float))	// r
					+ Aligned(sizeof(float))			// g
					+ Aligned(sizeof(float))			// b
					+ Aligned(sizeof(float));			// a
			}

			// TextRenderer
			if (entity.HasComponent<TextRendererComponent>())
			{
				TextRendererComponent trc = entity.GetComponent<TextRendererComponent>();

				requiredSize += Aligned(sizeof(uint64_t)) // Texture Asset UUID
					+ Aligned(sizeof(trc.KerningOffset))
					+ Aligned(sizeof(trc.LineHeightOffset))
					+ Aligned(sizeof(uint64_t))		// size of Text String
					+ Aligned(trc.Text.size() * sizeof(char));	// Text String

				// Color
				requiredSize += Aligned(sizeof(trc.TextColor.r))
					+ Aligned(sizeof(trc.TextColor.g))
					+ Aligned(sizeof(trc.TextColor.b))
					+ Aligned(sizeof(trc.TextColor.a));

				requiredSize += Aligned(sizeof(trc.BGColor.r))
					+ Aligned(sizeof(trc.BGColor.g))
					+ Aligned(sizeof(trc.BGColor.b))
					+ Aligned(sizeof(trc.BGColor.a));
			}

			// Rigidbody2D
			if (entity.HasComponent<Rigidbody2DComponent>())
			{
				requiredSize += Aligned(sizeof(uint16_t))	// Type
					+ Aligned(sizeof(bool));				// FixedRotation
			}

			// BoxCollider2D
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				requiredSize += Aligned(sizeof(bool))	// Show
					+ Aligned(sizeof(float))			// Density
					+ Aligned(sizeof(float))			// Friction
					+ Aligned(sizeof(float))			// Restitution
					+ Aligned(sizeof(float));			// RestitutionThreshold

				// Offset
				requiredSize += Aligned(sizeof(float))	// Offset.x
					+ Aligned(sizeof(float));			// Offset.y

				// Size
				requiredSize += Aligned(sizeof(float))	// Size.x
					+ Aligned(sizeof(float));			// Size.Y

			}

			// CircleCollider2D
			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				requiredSize += Aligned(sizeof(bool))	// Show
					+ Aligned(sizeof(float))				// Radius
					+ Aligned(sizeof(float))				// Density
					+ Aligned(sizeof(float))				// Friction
					+ Aligned(sizeof(float))				// Restitution
					+ Aligned(sizeof(float));				// RestitutionThreshold

				// Offset
				requiredSize += Aligned(sizeof(float))		// Offset.x
					+ Aligned(sizeof(float));				// Offset.y
			}

			// NativeScript
			if (entity.HasComponent<NativeScriptComponent>())
			{

			}

			// Script
			if (entity.HasComponent<ScriptComponent>())
			{
				ScriptComponent sc = entity.GetComponent<ScriptComponent>();

				requiredSize += Aligned(sizeof(uint64_t))		// Size of Class Name String
					+ Aligned(sc.ClassName.size() * sizeof(char));	// Class Name String
			}
		}

		// Allocate Size for Data
		entityInfo.DataBuffer.Allocate(requiredSize);

		// Set Data
		{
			if (entityInfo.DataBuffer.Data)
			{
				if (entityInfo.DataBuffer.Size >= requiredSize)
				{
					// Start at beginning of buffer
					uint8_t* destination = entityInfo.DataBuffer.Data;
					uint8_t* end = destination + requiredSize;

					// Clear requiredSize from destination
					memset(destination, 0, requiredSize);

					// Fill out buffer in order. Based on Size step.

					uint64_t uuid = entity.GetComponent<IDComponent>().ID;
					*(uint64_t*)destination = uuid;
					destination += Aligned(sizeof(uuid));

					// All Components
					{
						// Tag
						if (entity.HasComponent<TagComponent>())
						{
							TagComponent tc = entity.GetComponent<TagComponent>();
							size_t stringSize = tc.Tag.size();
							*(size_t*)destination = stringSize;
							destination += Aligned(sizeof(stringSize));
							memcpy(destination, tc.Tag.c_str(), stringSize * sizeof(char));
							destination += Aligned(tc.Tag.size() * sizeof(char));
						}

						// Transform
						if (entity.HasComponent<TransformComponent>())
						{
							TransformComponent trsc = entity.GetComponent<TransformComponent>();

							// Translation
							*(float*)destination = trsc.Translation.x;
							destination += Aligned(sizeof(trsc.Translation.x));

							*(float*)destination = trsc.Translation.y;
							destination += Aligned(sizeof(trsc.Translation.y));

							*(float*)destination = trsc.Translation.z;
							destination += Aligned(sizeof(trsc.Translation.z));

							// Rotation
							*(float*)destination = trsc.Rotation.x;
							destination += Aligned(sizeof(trsc.Rotation.x));

							*(float*)destination = trsc.Rotation.y;
							destination += Aligned(sizeof(trsc.Rotation.y));

							*(float*)destination = trsc.Rotation.z;
							destination += Aligned(sizeof(trsc.Rotation.z));

							// Scale
							*(float*)destination = trsc.Scale.x;
							destination += Aligned(sizeof(trsc.Scale.x));

							*(float*)destination = trsc.Scale.y;
							destination += Aligned(sizeof(trsc.Scale.y));

							*(float*)destination = trsc.Scale.z;
							destination += Aligned(sizeof(trsc.Scale.z));

						}

						// Camera
						if (entity.HasComponent<CameraComponent>())
						{
							CameraComponent cc = entity.GetComponent<CameraComponent>();

							*(bool*)destination = cc.Primary;
							destination += Aligned(sizeof(bool));

							*(bool*)destination = cc.FixedAspectRatio;
							destination += Aligned(sizeof(bool));

							// SceneCamera Variables
							*(float*)destination = cc.ActiveCamera.GetFOV();
							destination += Aligned(sizeof(float));

							*(float*)destination = cc.ActiveCamera.GetNearClip();
							destination += Aligned(sizeof(float));

							*(float*)destination = cc.ActiveCamera.GetFarClip();
							destination += Aligned(sizeof(float));

						}

						// AudioSource
						if (entity.HasComponent<AudioSourceComponent>())
						{
							AudioSourceComponent asc = entity.GetComponent<AudioSourceComponent>();

							*(uint64_t*)destination = asc.AssetHandle;
							destination += Aligned(sizeof(uint64_t));

							*(bool*)destination = asc.Loop;
							destination += Aligned(sizeof(bool));

							*(float*)destination = asc.Pitch;
							destination += Aligned(sizeof(float));

							*(float*)destination = asc.Gain;
							destination += Aligned(sizeof(float));

						}

						// AudioListener
						if (entity.HasComponent<AudioListenerComponent>())
						{
							AudioListenerComponent alc = entity.GetComponent<AudioListenerComponent>();

						}

						// SpriteRenderer
						if (entity.HasComponent<SpriteRendererComponent>())
						{
							SpriteRendererComponent src = entity.GetComponent<SpriteRendererComponent>();

							*(uint64_t*)destination = src.AssetHandle;
							destination += Aligned(sizeof(uint64_t));

							*(float*)destination = src.TilingFactor;
							destination += Aligned(sizeof(float));

							// Color
							*(float*)destination = src.Color.r;
							destination += Aligned(sizeof(float));

							*(float*)destination = src.Color.g;
							destination += Aligned(sizeof(float));

							*(float*)destination = src.Color.b;
							destination += Aligned(sizeof(float));

							*(float*)destination = src.Color.a;
							destination += Aligned(sizeof(float));

						}

						// CircleRenderer
						if (entity.HasComponent<CircleRendererComponent>())
						{
							CircleRendererComponent crc = entity.GetComponent<CircleRendererComponent>();

							*(uint64_t*)destination = crc.AssetHandle;
							destination += Aligned(sizeof(uint64_t));

							*(float*)destination = crc.TilingFactor;
							destination += Aligned(sizeof(float));

							*(float*)destination = crc.Radius;
							destination += Aligned(sizeof(float));

							*(float*)destination = crc.Thickness;
							destination += Aligned(sizeof(float));

							*(float*)destination = crc.Fade;
							destination += Aligned(sizeof(float));

							// Color
							*(float*)destination = crc.Color.r;
							destination += Aligned(sizeof(float));

							*(float*)destination = crc.Color.g;
							destination += Aligned(sizeof(float));

							*(float*)destination = crc.Color.b;
							destination += Aligned(sizeof(float));

							*(float*)destination = crc.Color.a;
							destination += Aligned(sizeof(float));
						}

						// TextRenderer
						if (entity.HasComponent<TextRendererComponent>())
						{
							TextRendererComponent trc = entity.GetComponent<TextRendererComponent>();

							*(uint64_t*)destination = trc.AssetHandle;
							destination += Aligned(sizeof(uint64_t));

							*(float*)destination = trc.KerningOffset;
							destination += Aligned(sizeof(float));
							*(float*)destination = trc.LineHeightOffset;
							destination += Aligned(sizeof(float));

							size_t stringSize = trc.Text.size();
							*(size_t*)destination = stringSize;
							destination += Aligned(sizeof(stringSize));
							memcpy(destination, trc.Text.c_str(), stringSize * sizeof(char));
							destination += Aligned(trc.Text.size() * sizeof(char));

							// Color
							*(float*)destination = trc.TextColor.r;
							destination += Aligned(sizeof(float));

							*(float*)destination = trc.TextColor.g;
							destination += Aligned(sizeof(float));

							*(float*)destination = trc.TextColor.b;
							destination += Aligned(sizeof(float));

							*(float*)destination = trc.TextColor.a;
							destination += Aligned(sizeof(float));

							*(float*)destination = trc.BGColor.r;
							destination += Aligned(sizeof(float));

							*(float*)destination = trc.BGColor.g;
							destination += Aligned(sizeof(float));

							*(float*)destination = trc.BGColor.b;
							destination += Aligned(sizeof(float));

							*(float*)destination = trc.BGColor.a;
							destination += Aligned(sizeof(float));
						}

						// Rigidbody2D
						if (entity.HasComponent<Rigidbody2DComponent>())
						{
							Rigidbody2DComponent rb2D = entity.GetComponent<Rigidbody2DComponent>();

							*(uint16_t*)destination = (uint16_t)rb2D.Type;
							destination += Aligned(sizeof(uint16_t));

							*(bool*)destination = rb2D.FixedRotation;
							destination += Aligned(sizeof(bool));
						}

						// BoxCollider2D
						if (entity.HasComponent<BoxCollider2DComponent>())
						{
							BoxCollider2DComponent bc2d = entity.GetComponent<BoxCollider2DComponent>();

							*(bool*)destination = bc2d.Show;
							destination += Aligned(sizeof(bool));

							*(float*)destination = bc2d.Density;
							destination += Aligned(sizeof(float));

							*(float*)destination = bc2d.Friction;
							destination += Aligned(sizeof(float));

							*(float*)destination = bc2d.Restitution;
							destination += Aligned(sizeof(float));

							*(float*)destination = bc2d.RestitutionThreshold;
							destination += Aligned(sizeof(float));

							// Offset
							*(float*)destination = bc2d.Offset.x;
							destination += Aligned(sizeof(float));

							*(float*)destination = bc2d.Offset.y;
							destination += Aligned(sizeof(float));

							// Size
							*(float*)destination = bc2d.Size.x;
							destination += Aligned(sizeof(float));

							*(float*)destination = bc2d.Size.y;
							destination += Aligned(sizeof(float));

						}

						// CircleCollider2D
						if (entity.HasComponent<CircleCollider2DComponent>())
						{
							CircleCollider2DComponent cc2d = entity.GetComponent<CircleCollider2DComponent>();

							*(bool*)destination = cc2d.Show;
							destination += Aligned(sizeof(bool));

							*(float*)destination = cc2d.Radius;
							destination += Aligned(sizeof(float));

							*(float*)destination = cc2d.Density;
							destination += Aligned(sizeof(float));

							*(float*)destination = cc2d.Friction;
							destination += Aligned(sizeof(float));

							*(float*)destination = cc2d.Restitution;
							destination += Aligned(sizeof(float));

							*(float*)destination = cc2d.RestitutionThreshold;
							destination += Aligned(sizeof(float));

							// Offset
							*(float*)destination = cc2d.Offset.x;
							destination += Aligned(sizeof(float));

							*(float*)destination = cc2d.Offset.y;
							destination += Aligned(sizeof(float));
						}

						// NativeScript
						if (entity.HasComponent<NativeScriptComponent>())
						{

						}

						// Script
						if (entity.HasComponent<ScriptComponent>())
						{
							ScriptComponent sc = entity.GetComponent<ScriptComponent>();

							size_t stringSize = sc.ClassName.size();
							*(size_t*)destination = stringSize;
							destination += Aligned(sizeof(stringSize));
							memcpy(destination, sc.ClassName.c_str(), stringSize * sizeof(char));
							destination += Aligned(sc.ClassName.size() * sizeof(char));
						}


					}

					if (destination - (uint8_t*)entityInfo.DataBuffer.Data == requiredSize)
					{
						GE_CORE_INFO("AssetSerializer::SerializeEntity(entity, info) Successful.");
						return true;
					}
					else
					{
						GE_CORE_ASSERT(false, "Buffer overflow.");
					}
				}
				else
				{
					GE_CORE_ERROR("AssetSerializer::SerializeEntity(entity, info) : Required size is larger than given buffer size.");
				}
			}
		}
		return false;
	}

	static bool DeserializeEntity(const SceneInfo::EntityInfo& eInfo, Entity& entity)
	{
		if (!eInfo.DataBuffer.Data)
			return false;

		// Read Data & Assign

		return true;
	}

#pragma endregion

#pragma region Font

	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> func>
	static Ref<Texture2D> CreateAndCacheFontAtlas(const AssetMetadata& metadata, const std::string& atlasName,
		Font::AtlasConfig& atlasConfig, Ref<Font::MSDFData> msdfData)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
		{
			GE_CORE_ERROR("Failed to load Font Freetype Handle");
			return 0;
		}

		std::filesystem::path path = Project::GetPathToAsset(metadata.FilePath);
		std::string fontString = path.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontString.c_str());

		if (!font)
		{
			GE_CORE_ERROR("Failed to load Font Atlas.");
			return 0;
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
		atlasConfig.Width = width;
		atlasConfig.Height = height;
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
	
		msdf_atlas::ImmediateAtlasGenerator<S, N, func, msdf_atlas::BitmapAtlasStorage<T, N>> atlasGenerator(atlasConfig.Width, atlasConfig.Height);
		atlasGenerator.setAttributes(attributes);
		atlasGenerator.setThreadCount(8);
		atlasGenerator.generate(msdfData->Glyphs.data(), (int)msdfData->Glyphs.size());
	
		msdfgen::BitmapConstRef<T, N> bitmap = atlasGenerator.atlasStorage();
		Texture::Config config;
		config.Height = bitmap.height;
		config.Width = bitmap.width;
		config.InternalFormat = Texture::ImageFormat::RGB8;
		config.Format = Texture::DataFormat::RGB;
		config.GenerateMips = false;
	
		Buffer dataBuffer((void*)bitmap.pixels, 
			bitmap.height * bitmap.width * (config.InternalFormat == Texture::ImageFormat::RGB8 ? 3 : 4));
		Ref<Texture2D> texture = Texture2D::Create(config, dataBuffer);

		return texture;
	}

#pragma endregion

#pragma region Audio

	static std::int32_t convert_to_int(char* audioClip, std::size_t len)
	{
		std::int32_t a = 0;
		std::memcpy(&a, audioClip, len);
		return a;
	}

	static  bool LoadWavFile(std::ifstream& file, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, uint64_t& size)
	{
		char audioClip[4];
		if (!file.is_open())
			return false;

		// the RIFF
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read RIFF while loading Wav file.");
			return false;
		}
		if (std::strncmp(audioClip, "RIFF", 4) != 0)
		{
			GE_CORE_ERROR("File is not a valid WAVE file (header doesn't begin with RIFF)");
			return false;
		}

		// the size of the file
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read size of Wav file.");
			return false;
		}

		// the WAVE
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read WAVE");
			return false;
		}
		if (std::strncmp(audioClip, "WAVE", 4) != 0)
		{
			GE_CORE_ERROR("File is not a valid WAVE file (header doesn't contain WAVE)");
			return false;
		}

		// "fmt/0"
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read fmt of Wav file.");
			return false;
		}

		// this is always 16, the size of the fmt data chunk
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read the size of the fmt data chunk. Should be 16.");
			return false;
		}

		// PCM should be 1?
		if (!file.read(audioClip, 2))
		{
			GE_CORE_ERROR("Could not read PCM. Should be 1.");
			return false;
		}

		// the number of Channels
		if (!file.read(audioClip, 2))
		{
			GE_CORE_ERROR("Could not read number of Channels.");
			return false;
		}
		channels = convert_to_int(audioClip, 2);

		// sample rate
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read sample rate.");
			return false;
		}
		sampleRate = convert_to_int(audioClip, 4);

		// (SampleRate * BPS * Channels) / 8
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read (SampleRate * BPS * Channels) / 8");
			return false;
		}

		// ?? dafaq
		if (!file.read(audioClip, 2))
		{
			GE_CORE_ERROR("Could not read dafaq?");
			return false;
		}

		// BPS
		if (!file.read(audioClip, 2))
		{
			GE_CORE_ERROR("Could not read bits per sample.");
			return false;
		}
		bitsPerSample = convert_to_int(audioClip, 2);

		// data chunk header "data"
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read data chunk header.");
			return false;
		}
		if (std::strncmp(audioClip, "data", 4) != 0)
		{
			GE_CORE_ERROR("File is not a valid WAVE file (doesn't have 'data' tag).");
			return false;
		}

		// size of data
		if (!file.read(audioClip, 4))
		{
			GE_CORE_ERROR("Could not read data size.");
			return false;
		}
		size = convert_to_int(audioClip, 4);

		/* cannot be at the end of file */
		if (file.eof())
		{
			GE_CORE_ERROR("Reached EOF.");
			return false;
		}
		if (file.fail())
		{
			GE_CORE_ERROR("Fail state set on the file.");
			return false;
		}

		return true;
	}

	static bool LoadWav(const std::filesystem::path& filePath, Ref<AudioBuffer> audioBuffer)
	{
		std::ifstream stream(filePath, std::ios::binary);
		if (!stream.is_open())
		{
			GE_CORE_ERROR("Could not open WAV file {0}", filePath.string().c_str());
			return false;
		}
		if (!LoadWavFile(stream, audioBuffer->Channels, audioBuffer->SampleRate, audioBuffer->BPS, audioBuffer->Buff.Size))
		{
			GE_CORE_ERROR("Could not load WAV file {0}", filePath.string().c_str());
			return false;
		}

		if (audioBuffer->Channels == 1 && audioBuffer->BPS == 8)
			audioBuffer->Format = AL_FORMAT_MONO8;
		else if (audioBuffer->Channels == 1 && audioBuffer->BPS == 16)
			audioBuffer->Format = AL_FORMAT_MONO16;
		else if (audioBuffer->Channels == 2 && audioBuffer->BPS == 8)
			audioBuffer->Format = AL_FORMAT_STEREO8;
		else if (audioBuffer->Channels == 2 && audioBuffer->BPS == 16)
			audioBuffer->Format = AL_FORMAT_STEREO16;
		else
		{
			GE_CORE_ERROR("Unrecognized wave format.\nChannels {0}\nBPS {1}\n", audioBuffer->Channels, audioBuffer->BPS);
			return 0;
		}

		for (uint32_t i = 0; i < audioBuffer->NUM_BUFFERS; i++)
		{
			uint8_t* soundData = new uint8_t[audioBuffer->Buff.Size];
			stream.read((char*)soundData, audioBuffer->Buff.Size);
			audioBuffer->Buff.Data += *soundData;
		}

		stream.close();
		return true;
	}

#pragma endregion

	bool AssetSerializer::SerializeRegistry(Ref<AssetRegistry> registry)
	{
		// gar(Game Asset Registry) file
		GE_CORE_INFO("Asset Registry Serialization Started.");

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;
			GE_CORE_TRACE("Serializing AssetRegistry\n\tFilePath : {0}", registry->m_FilePath.string().c_str());

			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : registry->GetRegistry())
			{
				GE_CORE_TRACE("Serializing Asset\n\tUUID : {0}\n\tFilePath : {1}\n\tType : {2}", (uint64_t)handle, metadata.FilePath.string().c_str(), AssetUtils::AssetTypeToString(metadata.Type));
				
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
		std::filesystem::path path = Project::GetPathToAsset(registry->m_FilePath);
		std::ofstream fout(path);

		if (fout.is_open() && fout.good())
		{
			fout << out.c_str();
			fout.close();
			GE_CORE_INFO("Asset Registry Serialization Complete.");
			return true;
		}

		GE_CORE_WARN("Asset Registry Serialization Failed.");
		return false;
	}

	bool AssetSerializer::DeserializeRegistry(Ref<AssetRegistry> registry)
	{
		const std::filesystem::path& path = Project::GetPathToAsset(registry->m_FilePath);
		if (path.empty())
			return false;
		GE_CORE_INFO("Asset Registry Deserialization Started.\n\tFilePath : {0}", path.string().c_str());

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load Asset Registry file. {0}\n\t{1}", path.string().c_str(), e.what());
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
			UUID handle = node["Handle"].as<uint64_t>();
			AssetMetadata assetMetadata;
			assetMetadata.Handle = handle;
			assetMetadata.FilePath = node["FilePath"].as<std::string>();
			assetMetadata.Type = AssetUtils::AssetTypeFromString(node["Type"].as<std::string>());
			GE_CORE_TRACE("Deserializing Asset\n\tUUID : {0}\n\tFilePath : {1}\n\tType : {2}", (uint64_t)assetMetadata.Handle, assetMetadata.FilePath.string().c_str(), AssetUtils::AssetTypeToString(assetMetadata.Type));
			registry->AddAsset(assetMetadata);
		}
		GE_CORE_INFO("Asset Registry Deserialization Complete.");
		return true;
	}

	bool AssetSerializer::SerializePack(Ref<AssetPack> pack)
	{
		// gap(Game Asset Pack) file
		//  [bytes]
		
		//	[16] header, info
		//  [4] signature // File Extension "GAP"
		//  [4] Version // File Format Version
		//  [8] Build Version // Date/Time Built

		//	[100 + ?] index, data. Size depends on Assets.
		//  [8] Offset	// Start of index relative to SOF(start of file), header Size
		//  [8] Size	// Size of all Scenes, Count & Map
		//	[8] Scene Map Count
		//  [76 + ?] Scene Map			// Size based on how many Scenes are loaded
		//      [8] Asset Handle    : Key
		//      [68 + ?] SceneInfo  : Value
		//          [8] Packed Size : Size of whole Scene
		//			[60 + ?] Data
		//				[2] Type
		//				[8] Name
		//				[8] Asset Map Count
		//				[26 + ?] Asset Map			// Size based on how many Assets are loaded
		//				    [8] Asset Handle    : Key
		//				    [18 + ?] AssetInfo	: Value
		//				        [8] Packed Size
		//						[10 + ?] Packed Data
		//							[2] Type
		//							[8] Name
		//				[8] Entity Map Count
		//				[16 + ?] Entity Map			// Size based on how many Entities are loaded
		//				    [8] Handle			: Key
		//				    [8 + ?] EntityInfo	: Value
		//				        [8] Packed Size
		//						[?]	Packed Data

		GE_CORE_TRACE("Asset Pack Serialization Started");
		std::ofstream stream(Project::GetPathToAsset(pack->m_File.Path), std::ios::out | std::ios::binary);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Pack file to write.");
			return false;
		}

		// Header
		{
			stream.write(reinterpret_cast<const char*>(pack->m_File.FileHeader.HEADER), sizeof(pack->m_File.FileHeader.HEADER));
			stream.write(reinterpret_cast<const char*>(&pack->m_File.FileHeader.Version),
				sizeof(pack->m_File.FileHeader.Version));
			stream.write(reinterpret_cast<const char*>(&pack->m_File.FileHeader.BuildVersion),
				sizeof(pack->m_File.FileHeader.BuildVersion));
		}
			
		// Index
		{
			// Set Index.Offset based on Header.Size
			pack->m_File.Index.Offset = pack->GetHeaderSize();
			stream.write(reinterpret_cast<const char*>(&pack->m_File.Index.Offset), sizeof(pack->m_File.Index.Offset));

			for (const auto& [uuid, asset] : Project::GetAssetManager<RuntimeAssetManager>()->GetLoadedAssets())
			{
				if (asset->GetType() == Asset::Type::Scene)
				{
					if (SerializeAsset(asset, pack->m_File.Index.Scenes[uuid]))
					{
						// Scene and its children have been populated into pack->m_File.Index.Scenes.at(uuid)
						// Scene.Size should represent the total scene size and be aligned
						// Add to Index.Size for each Scene Asset
						pack->m_File.Index.Size += pack->m_File.Index.Scenes.at(uuid).DataBuffer.Size;
					}
				}
			}

			// Index.Size contains
			// - Scene Count
			// - All Scenes
			pack->m_File.Index.Size += Aligned(sizeof(pack->m_File.Index.Scenes.size()));
			stream.write(reinterpret_cast<const char*>(&pack->m_File.Index.Size), sizeof(pack->m_File.Index.Size));

			uint64_t allScenesCount = pack->m_File.Index.Scenes.size();
			stream.write(reinterpret_cast<const char*>(&allScenesCount), sizeof(uint64_t));
			for (const auto& [sceneHandle, sceneInfo] : pack->m_File.Index.Scenes)
			{
				// Write all data
				// SceneInfo.Data contains all AssetInfo & EntityInfo
				stream.write(reinterpret_cast<const char*>(&sceneInfo.DataBuffer.Size), sizeof(sceneInfo.DataBuffer.Size));
				stream.write(reinterpret_cast<const char*>(sceneInfo.DataBuffer.Data), sceneInfo.DataBuffer.Size);

			}
		}
		
		stream.close();
		GE_CORE_TRACE("Asset Pack Serialization Complete");
		return stream.good();
	}

	// TODO:
	bool AssetSerializer::DeserializePack(Ref<AssetPack> pack)
	{
		// gap(Game Asset Pack) file
		//  [bytes]

		//	[16] header, info
		//  [4] signature // File Extension "GAP"
		//  [4] Version // File Format Version
		//  [8] Build Version // Date/Time Built

		//	[100 + ?] index, data. Size depends on Assets & Entities.
		//  [8] Offset	// Start of index relative to SOF(start of file), header Size
		//  [8] Size	// Size of all Scenes, Count & Map
		//	[8] Scene Map Count
		//  [76 + ?] Scene Map			// Size based on how many Scenes are loaded
		//      [8] Asset Handle    : Key
		//      [68 + ?] SceneInfo  : Value
		//          [8] Packed Size : Size of whole Scene
		//			[60 + ?] Data
		//				[2] Type
		//				[8] Name
		//				[8] Asset Map Count
		//				[26 + ?] Asset Map			// Size based on how many Assets are loaded
		//				    [8] Asset Handle    : Key
		//				    [18 + ?] AssetInfo	: Value
		//				        [8] Packed Size
		//						[10 + ?] Packed Data
		//							[2] Type
		//							[8] Name
		//				[8] Entity Map Count
		//				[16 + ?] Entity Map			// Size based on how many Entities are loaded
		//				    [8] Handle			: Key
		//				    [8 + ?] EntityInfo	: Value
		//				        [8] Packed Size
		//						[?]	Packed Data

		std::ifstream stream(Project::GetPathToAsset(pack->m_File.Path), std::ios::in | std::ios::binary);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Pack file to read.");
			return false;
		}

		// Header
		{
			stream.read((char*)&pack->m_File.FileHeader.HEADER, sizeof(pack->m_File.FileHeader.HEADER));
			stream.read((char*)&pack->m_File.FileHeader.Version, sizeof(pack->m_File.FileHeader.Version));
			stream.read((char*)&pack->m_File.FileHeader.BuildVersion, sizeof(pack->m_File.FileHeader.BuildVersion));
		}

		// Index
		{
			stream.read((char*)&pack->m_File.Index.Offset, sizeof(pack->m_File.Index.Offset));
			stream.read((char*)&pack->m_File.Index.Size, sizeof(pack->m_File.Index.Size));
			uint64_t sceneCount = 0;
			stream.read((char*)&sceneCount, sizeof(sceneCount));
			for (uint64_t i = 0; i < sceneCount; i++)
			{
				SceneInfo sceneInfo = SceneInfo();
				stream.read((char*)&sceneInfo.DataBuffer.Size, sizeof(sceneInfo.DataBuffer.Size));
				stream.read((char*)&sceneInfo.DataBuffer.Data, sceneInfo.DataBuffer.Size);

				if (Ref<Asset> sceneAsset = DeserializeAsset(sceneInfo))
				{
					Project::GetAssetManager()->AddAsset(sceneAsset);
					pack->AddAsset(sceneAsset, sceneInfo);

					// Assets & Entities
					{
						for (const auto& [uuid, childAssetInfo] : sceneInfo.m_Assets)
						{
							if (Ref<Asset> childAsset = DeserializeAsset(childAssetInfo))
							{
								Project::GetAssetManager()->AddAsset(childAsset);
								pack->AddAsset(childAsset, childAssetInfo);
							}
						}

						Ref<Scene> scene = Project::GetAsset<Scene>(sceneAsset->GetHandle());
						for (const auto& [uuid, entityInfo] : sceneInfo.m_Entities)
						{
							Entity entity = Entity();
							if (DeserializeEntity(entityInfo, entity))
							{
								scene->CopyEntity(entity);
							}
						}
					}

				}

			}
		}

		stream.close();
		return stream.good();
	}

	Ref<Asset> AssetSerializer::DeserializeAsset(const AssetMetadata& metadata)
	{
		if (s_AssetDeserializeFuncs.find(metadata.Type) == s_AssetDeserializeFuncs.end())
		{
			GE_CORE_ERROR("Deserialize metadata function not found for Type: {0}", AssetUtils::AssetTypeToString(metadata.Type));
			return nullptr;
		}
		return s_AssetDeserializeFuncs.at(metadata.Type)(metadata);
	}

	Ref<Asset> AssetSerializer::DeserializeAsset(const AssetInfo& assetInfo)
	{
		Asset::Type assetType = (Asset::Type)assetInfo.Type;
		if (s_AssetPackDeserializeFuncs.find(assetType) == s_AssetPackDeserializeFuncs.end())
		{
			GE_CORE_ERROR("Deserialize function not found for Type: " + AssetUtils::AssetTypeToString(assetType));
			return nullptr;
		}
		return s_AssetPackDeserializeFuncs.at(assetType)(assetInfo);
	}

	bool AssetSerializer::SerializeAsset(Ref<Asset> asset, const AssetMetadata& metadata)
	{
		if (s_AssetSerializeFuncs.find(metadata.Type) == s_AssetSerializeFuncs.end())
		{
			GE_CORE_ERROR("Serialize metadata function not found for Type: " + AssetUtils::AssetTypeToString(metadata.Type));
			return false;
		}
		return s_AssetSerializeFuncs.at(metadata.Type)(asset, metadata);
	}

	bool AssetSerializer::SerializeAsset(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		if (s_AssetPackSerializeFuncs.find(asset->GetType()) == s_AssetPackSerializeFuncs.end())
		{
			GE_CORE_ERROR("Serialize asset info function not found for Type: " + AssetUtils::AssetTypeToString(asset->GetType()));
			return false;
		}
		return s_AssetPackSerializeFuncs.at(asset->GetType())(asset, assetInfo);
	}

	Ref<Asset> AssetSerializer::DeserializeScene(const AssetMetadata& metadata)
	{
		Ref<Scene> scene = CreateRef<Scene>(metadata.Handle);
		scene->p_Status = Asset::Status::Loading;

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
			GE_CORE_ERROR("Failed to load Scene Asset file. {0}\n\t{1}", path.string(), e.what());
		}

		if (!data["Scene"])
		{
			scene->p_Status = Asset::Status::Invalid;
			return scene;
		}

		std::string sceneName = data["Scene"].as<std::string>();
		GE_CORE_TRACE("Deserializing Scene\n\tUUID : {0}\n\tName : {1}\n\tPath : {2}", (uint64_t)metadata.Handle, sceneName.c_str(), path.string());
		scene->m_Config.Name = sceneName;

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				// ID Component 
				uint64_t uuid = entity["Entity"].as<uint64_t>(); // UUID

				// TagComponent
				std::string name = std::string();
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				GE_CORE_TRACE("Deserializing entity\n\tUUID : {0},\n\tName : {1}", uuid, name);

				Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name);
				if (DeserializeEntity(entity, deserializedEntity))
					GE_CORE_INFO("Entity Deserialization Successful.");
			}
		}
		
		scene->p_Status = Asset::Status::Ready;
		return scene;
	}

	// TODO:
	Ref<Asset> AssetSerializer::DeserializeSceneFromPack(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer.Size == 0 || !assetInfo.DataBuffer.Data)
		{
			GE_CORE_ERROR("Cannot import Scene from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		Ref<Scene> scene = CreateRef<Scene>();
		
		// Read Data & Assign

		return scene;
	}

	Ref<Asset> AssetSerializer::DeserializeTexture2D(const AssetMetadata& metadata)
	{
		int width = 0, height = 0, channels = 4;
		stbi_set_flip_vertically_on_load(1);
		Buffer data;
		{
			GE_PROFILE_SCOPE("stbi_load - AssetSerializer::DeserializeTexture2D");
			std::string path = Project::GetPathToAsset(metadata.FilePath).string();
			data.Data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		GE_CORE_ASSERT(data, "Failed to load stb image!");
		data.Size = width * height * channels;  // Assumed 1 byte per channel

		Texture::Config config;
		config.Height = height;
		config.Width = width;
		if (channels == 3)
		{
			config.InternalFormat = Texture::ImageFormat::RGB8;
			config.Format = Texture::DataFormat::RGB;
		}
		else if (channels == 4)
		{
			config.InternalFormat = Texture::ImageFormat::RGBA8;
			config.Format = Texture::DataFormat::RGBA;
		}
		else
			GE_CORE_WARN("Unsupported Texture2D Channels.");

		Ref<Texture2D> texture = Texture2D::Create(config, data);
		texture->p_Handle = metadata.Handle;
		texture->GetConfig().Name = metadata.FilePath.filename().string();

		data.Release();
		return texture;
	}

	// TODO:
	Ref<Asset> AssetSerializer::DeserializeTexture2DFromPack(const AssetInfo& assetInfo)
	{
		GE_CORE_ERROR("Cannot import Texture2D from AssetPack");
		return nullptr;
		/*
		Buffer data;
		TextureConfiguration config;
		Ref<Texture2D> texture = Texture2D::Create(config, data);
		data.Release();

		asset = texture;
		return true;
		*/
	}

	Ref<Asset> AssetSerializer::DeserializeFont(const AssetMetadata& metadata)
	{
		Ref<Font> asset = CreateRef<Font>(metadata.Handle);
		asset->p_Status = Asset::Status::Loading;
		asset->m_AtlasConfig.Texture =  CreateAndCacheFontAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>
			(metadata, "FontAtlas", asset->m_AtlasConfig, asset->m_MSDFData);

		asset->p_Status = Asset::Status::Ready;
		return asset;
	}

	// TODO:
	Ref<Asset> AssetSerializer::DeserializeFontFromPack(const AssetInfo& assetInfo)
	{
		GE_CORE_ERROR("Cannot import Font from AssetPack");
		return nullptr;
	}

	// TODO: Fix audio
	Ref<Asset> AssetSerializer::DeserializeAudio(const AssetMetadata& metadata)
	{
		Ref<AudioClip> audioSource = CreateRef<AudioClip>();
		audioSource->p_Handle = metadata.Handle;

		if (LoadWav(Project::GetPathToAsset(metadata.FilePath), audioSource->m_AudioBuffer))
		{
			alGenBuffers(audioSource->m_AudioBuffer->NUM_BUFFERS, audioSource->m_AudioBuffer->Buffers);

			for (std::size_t i = 0; i < audioSource->m_AudioBuffer->NUM_BUFFERS; ++i)
			{
				alBufferData(audioSource->m_AudioBuffer->Buffers[i], 
					audioSource->m_AudioBuffer->Format, 
					&audioSource->m_AudioBuffer->Buff.Data[i], audioSource->m_AudioBuffer->Buff.Size,
					audioSource->m_AudioBuffer->SampleRate);

				ALenum error = alGetError();
				if (error != AL_NO_ERROR)
				{
					GE_CORE_ERROR("OpenAL Error: {0}", (char*)alGetString(error));
					if (audioSource->m_AudioBuffer->Buffers[i] && alIsBuffer(audioSource->m_AudioBuffer->Buffers[i]))
						alDeleteBuffers(1, &audioSource->m_AudioBuffer->Buffers[i]);
					return 0;
				}
			}
				
		}

		return audioSource;
	}

	// TODO: do last, fix Audio from metadata first
	Ref<Asset> AssetSerializer::DeserializeAudioFromPack(const AssetInfo& assetInfo)
	{
		GE_CORE_ERROR("Cannot import Audio from AssetPack");
		return nullptr;
	}
	
	bool AssetSerializer::SerializeScene(Ref<Asset> asset, const AssetMetadata& metadata)
	{
		std::filesystem::path path = Project::GetPathToAsset(metadata.FilePath);
		Ref<Scene> scene = Project::GetAssetAs<Scene>(asset);
		GE_CORE_TRACE("Serializing Scene\n\tUUID : {0}\n\tName : {1}\n\tPath : {2}", (uint64_t)metadata.Handle, scene->GetName().c_str(), path.string());

		YAML::Emitter out;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Scene" << YAML::Value << scene->GetName();
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

			scene->m_Registry.each([&](auto entityID)
				{
					Entity entity(entityID, scene.get());
					SerializeEntity(out, entity);
				});

			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		std::ofstream fout(path);
		if (fout.is_open() && fout.good())
		{
			fout << out.c_str();
			fout.close();
			GE_CORE_TRACE("Scene Serialization Complete.");
			return true;
		}
		
		GE_CORE_WARN("Scene Serialization Failed.");
		return false;
	}

	bool AssetSerializer::SerializeSceneForPack(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		//		[68 + ?] SceneInfo  : Value, corresponding Key handled in SerializePack
		//          [8] Packed Size : Size of Scene Buffer(bufferSize)
		//			[60 + ?] Data	: Scene Buffer
		//				[8] Handle
		//				[2] Type
		//				[8] Name Length
		//				[8] Name String
		//				[8] Steps
		//				[8] Asset Map Count
		//				[26 + ?] Asset Map			// Size based on how many Assets are loaded
		//						[8] Handle			: Key
		//						[18 + ?] AssetInfo	: Value
		//				        [8] Packed Size
		//						[10 + ?] Packed Data
		//							[8] Handle
		//							[2] Type
		//							[8] Name
		//							[?] Asset Specific Data
		//				[8] Entity Map Count
		//				[16 + ?] Entity Map			// Size based on how many Entities are loaded
		//				    [8] Handle			: Key
		//				    [8 + ?] EntityInfo	: Value
		//				        [8] Packed Size
		//						[?]	Packed Data
		//							[8] Handle
		//							[8] Name
		//							[?] Components
		
		Ref<Scene> scene = Project::GetAssetAs<Scene>(asset);
		if (!scene)
			return false;
		SceneInfo sceneInfo = assetInfo;
		uint64_t requiredSize = 0;

		// Calculate Size
		{
			requiredSize += Aligned(sizeof(scene->p_Handle)) // sizeof(uint64_t)
				+ Aligned(sizeof(scene->p_Type))	// sizeof(uint16_t)
				+ Aligned(sizeof(uint64_t)) // sizeof(stringLength)
				+ Aligned(scene->m_Config.Name.size() * sizeof(char)) //sizeof(string)
				+ Aligned(sizeof(scene->m_Config.StepFrames)); // sizeof(uint64_t)

			// For Assets
			{
				AssetMap assetMap = Project::GetAssetManager<RuntimeAssetManager>()->GetLoadedAssets();
				requiredSize += Aligned(sizeof(assetMap.size())); //sizeof(assetCount)
				for (const auto& [uuid, asset] : assetMap)
				{
					if (uuid == scene->p_Handle || asset->GetType() == Asset::Type::Scene)
						continue;

					if (SerializeAsset(asset, sceneInfo.m_Assets[uuid]))
					{
						requiredSize += Aligned(sizeof(uint64_t)) + Aligned(sceneInfo.m_Assets.at(uuid).DataBuffer.Size * sizeof(uint8_t));
					}
				}
			}

			// For Entities
			{
				requiredSize += Aligned(sizeof(uint64_t)); //sizeof(entityCount)
				{
					// Every Entity needs an ID
					auto idView = scene->GetRegistry().view<IDComponent>();
					for (auto e : idView)
					{
						Entity entity(e, scene.get());
						UUID uuid = entity.GetComponent<IDComponent>().ID;
						if (SerializeEntity(sceneInfo.m_Entities[uuid], entity))
						{
							requiredSize += Aligned(sizeof(uint64_t)) + Aligned(sceneInfo.m_Entities.at(uuid).DataBuffer.Size * sizeof(uint8_t));
						}

					}

				}

			}
		}

		// Allocate Size for Data
		assetInfo.InitializeData(requiredSize);

		// Set Data
		{
				if (assetInfo.DataBuffer.Data)
				{
					if (assetInfo.DataBuffer.Size >= requiredSize)
					{
						// Start at beginning of buffer
						uint8_t* destination = assetInfo.DataBuffer.Data;
						uint8_t* end = destination + requiredSize;

						// Clear requiredSize from destination
						memset(destination, 0, requiredSize);

						// Fill out buffer
						{
							*(uint64_t*)destination = scene->p_Handle;
							destination += Aligned(sizeof(uint64_t));

							*(uint16_t*)destination = (uint16_t)scene->p_Type;
							destination += Aligned(sizeof(uint16_t));

							// string
							size_t stringSize = scene->m_Config.Name.size();
							*(size_t*)destination = stringSize;
							destination += Aligned(sizeof(stringSize));
							memcpy(destination, scene->m_Config.Name.c_str(), stringSize * sizeof(char));
							destination += Aligned(scene->m_Config.Name.size() * sizeof(char));

							*(uint64_t*)destination = (uint64_t)scene->m_Config.StepFrames;
							destination += Aligned(sizeof(uint64_t));

							// For Assets
							{
								*(uint64_t*)destination = sceneInfo.m_Assets.size();
								destination += Aligned(sizeof(uint64_t));

								for (const auto& [uuid, assetInfo] : sceneInfo.m_Assets)
								{
									// size of asset data
									*(uint64_t*)destination = assetInfo.DataBuffer.Size;
									destination += Aligned(sizeof(uint64_t));

									// asset data
									memcpy(destination, assetInfo.DataBuffer.Data, assetInfo.DataBuffer.Size * sizeof(uint8_t));
									destination += Aligned(assetInfo.DataBuffer.Size * sizeof(uint8_t));

								}

							}

							// For Entities
							{
								*(size_t*)destination = sceneInfo.m_Entities.size();
								destination += Aligned(sizeof(size_t));

								for (const auto& [uuid, entityInfo] : sceneInfo.m_Entities)
								{
									// size of asset data
									*(uint64_t*)destination = entityInfo.DataBuffer.Size;
									destination += Aligned(sizeof(uint64_t));

									// asset data
									memcpy(destination, entityInfo.DataBuffer.Data, entityInfo.DataBuffer.Size * sizeof(uint8_t));
									destination += Aligned(entityInfo.DataBuffer.Size * sizeof(uint8_t));
								}
							}
						}

						if (destination - assetInfo.DataBuffer.Data == requiredSize)
						{
							GE_CORE_INFO("AssetSerializer::SerializeSceneForPack Successful.");
						}
						else
						{
							GE_CORE_ASSERT(false, "Buffer overflow.");
						}
					}
					else
					{
						GE_CORE_ERROR("Required size is larger than given buffer size.");
						return 0;
					}
			}

		}

		return true;

	}

	bool AssetSerializer::SerializeTexture2DForPack(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		/*
		* - Handle
		* - Type
		* - Name
		* ~ Config
		* * - Width
		* * - Height
		* * - InternalFormat
		* * - Format
		* * - GenerateMips
		* ~ Buffer
		* * - Size
		* * - Data
		*/

		Ref<Texture2D> texture = Project::GetAssetAs<Texture2D>(asset);
		assetInfo.Type = 2; // See Asset::Type
		uint64_t requiredSize = 0;

		// Size
		{
			requiredSize = Aligned(sizeof(uint64_t)) // Handle
				+ Aligned(sizeof(uint16_t))	// Type
				+ Aligned(sizeof(size_t)) // sizeof(stringLength)
				+ Aligned(texture->GetConfig().Name.size() * sizeof(char)); //sizeof(string)

			// Config
			requiredSize += Aligned(sizeof(uint32_t)) // Width
				+ Aligned(sizeof(uint32_t))  // Height
				+ Aligned(sizeof(uint16_t)) // InternalFormat
				+ Aligned(sizeof(uint16_t))	// Format
				+ Aligned(sizeof(bool)); // GenerateMips

			// Buffer Size & Data
			requiredSize += Aligned(sizeof(uint64_t)) + Aligned(texture->GetData().Size * sizeof(uint8_t));
		}

		// Allocate Size for Data
		assetInfo.InitializeData(requiredSize);

		// Data
		if (assetInfo.DataBuffer.Data)
		{
			if (assetInfo.DataBuffer.Size >= requiredSize)
			{
				// Start at beginning of buffer
				uint8_t* destination = assetInfo.DataBuffer.Data;
				uint8_t* end = destination + requiredSize;

				// Clear requiredSize from destination
				memset(destination, 0, requiredSize);

				// Fill out buffer
				{
					*(uint64_t*)destination = texture->p_Handle;
					destination += Aligned(sizeof(uint64_t));

					*(uint16_t*)destination = (uint16_t)texture->p_Type;
					destination += Aligned(sizeof(uint16_t));

					size_t stringSize = texture->GetConfig().Name.size();
					*(size_t*)destination = stringSize;
					destination += Aligned(sizeof(stringSize));
					memcpy(destination, texture->GetConfig().Name.c_str(), stringSize * sizeof(char));
					destination += Aligned(stringSize * sizeof(char));

					// Texture::Config

					*(uint32_t*)destination = texture->GetConfig().Width;
					destination += Aligned(sizeof(uint32_t));

					*(uint32_t*)destination = texture->GetConfig().Height;
					destination += Aligned(sizeof(uint32_t));

					*(uint16_t*)destination = (uint16_t)texture->GetConfig().InternalFormat;
					destination += Aligned(sizeof(uint16_t));

					*(uint16_t*)destination = (uint16_t)texture->GetConfig().Format;
					destination += Aligned(sizeof(uint16_t));

					*(bool*)destination = texture->GetConfig().GenerateMips;
					destination += Aligned(sizeof(bool));

					// Texture Buffer Size & Data

					*(uint64_t*)destination = texture->GetData().Size;
					destination += Aligned(sizeof(uint64_t));

					memcpy(destination, texture->GetData().Data, texture->GetData().Size * sizeof(uint8_t));
					destination += Aligned(texture->GetData().Size * sizeof(uint8_t));
				}

				if (destination - assetInfo.DataBuffer.Data == requiredSize)
				{
					GE_CORE_INFO("AssetSerializer::SerializeTexture2DForPack() Successful.");
					return true;
				}
				else
				{
					GE_CORE_ASSERT(false, "Buffer overflow.");
				}
			}
			else
			{
				GE_CORE_ERROR("Required size is larger than given buffer size.");
				return 0;
			}
		}
		return false;
	}

	bool AssetSerializer::SerializeFontForPack(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		/*
		* - Handle
		* - Type
		* - Name
		* ~ Config
		* * ~ Texture
		* * * - Type
		* * * - Name
		* * * ~ Config
		* * * * - Width
		* * * * - Height
		* * * * - InternalFormat
		* * * * - Format
		* * * * - GenerateMips
		* * * ~ Buffer
		* * * * - Size
		* * * * - Data
		* * - Width : uint32_t
		* * - Height : uint32_t
		* * - Scale : float
		* * - Seed : uint64_t
		* * - ThreadCount : int32_t
		* * - ExpensiveColoring : bool
		* - MSDFData
		* * - Glyphs : std::vector<msdf_atlas::GlyphGeometry>
		* * - Geometry : msdf_atlas::FontGeometry
		*/

		Ref<Font> font = Project::GetAssetAs<Font>(asset);
		assetInfo.Type = 3; // See Asset::Type
		AssetInfo atlasTextureInfo = AssetInfo();

		uint64_t requiredSize = 0;

		// Size
		{
			requiredSize = Aligned(sizeof(uint64_t))					// Handle
				+ Aligned(sizeof(uint16_t))									// Type
				+ Aligned(sizeof(uint64_t))									// Name string length
				+ Aligned(font->m_AtlasConfig.Name.size() * sizeof(char));	// Name string

			// Font Config
			requiredSize += Aligned(sizeof(uint32_t))	// Width
				+ Aligned(sizeof(uint32_t))				// Height
				+ Aligned(sizeof(float))				// Scale
				+ Aligned(sizeof(uint64_t))				// Seed
				+ Aligned(sizeof(uint32_t))				// Thread Count
				+ Aligned(sizeof(bool));				// ExpensiveColoring

			/*if (font->GetAtlasTexture())
			{
				if (SerializeAsset(font->GetAtlasTexture(), atlasTextureInfo))
					requiredSize += Aligned(sizeof(uint64_t)) + Aligned(atlasTextureInfo.DataBuffer.Size * sizeof(uint8_t));
			}*/

			// TODO
			// MSDFData
			//requiredSize += 0;
		}

		// Allocate Size for Data
		assetInfo.InitializeData(requiredSize);

		// Data
		if (assetInfo.DataBuffer.Data)
		{
			if (assetInfo.DataBuffer.Size >= requiredSize)
			{

				// Start at beginning of buffer
				uint8_t* destination = assetInfo.DataBuffer.Data;
				uint8_t* end = destination + requiredSize;

				// Clear requiredSize from destination
				memset(destination, 0, requiredSize);

				// Fill out buffer

				*(uint64_t*)destination = font->p_Handle;
				destination += Aligned(sizeof(uint64_t));

				*(uint16_t*)destination = (uint16_t)font->p_Type;
				destination += Aligned(sizeof(uint16_t));

				// Font Name
				uint64_t stringSize = font->m_AtlasConfig.Name.size();
				*(uint64_t*)destination = stringSize;
				destination += Aligned(sizeof(stringSize));
				memcpy(destination, font->m_AtlasConfig.Name.c_str(), stringSize * sizeof(char));
				destination += Aligned(font->m_AtlasConfig.Name.size() * sizeof(char));

				// Config
				{

					// Width
					*(uint32_t*)destination = font->m_AtlasConfig.Width;
					destination += Aligned(sizeof(uint32_t));

					// Height
					*(uint32_t*)destination = font->m_AtlasConfig.Height;
					destination += Aligned(sizeof(uint32_t));

					// Scale
					*(float*)destination = font->m_AtlasConfig.Scale;
					destination += Aligned(sizeof(float));

					// Seed
					*(uint64_t*)destination = font->m_AtlasConfig.Seed;
					destination += Aligned(sizeof(uint64_t));

					// ThreadCount
					*(int32_t*)destination = font->m_AtlasConfig.ThreadCount;
					destination += Aligned(sizeof(int32_t));

					// ExpensiveColoring
					*(bool*)destination = font->m_AtlasConfig.ExpensiveColoring;
					destination += Aligned(sizeof(bool));

					// Texture
					//if (font->GetAtlasTexture())
					//{
					//	// size of asset data
					//	*(uint64_t*)destination = atlasTextureInfo.DataBuffer.Size;
					//	destination += Aligned(sizeof(uint64_t));

					//	// asset data
					//	memcpy(destination, atlasTextureInfo.DataBuffer.Data, atlasTextureInfo.DataBuffer.Size * sizeof(uint8_t));
					//	destination += Aligned(atlasTextureInfo.DataBuffer.Size * sizeof(uint8_t));
					//}

				}

				// MSDFData
				{
					// TODO
					//msdf_atlas::GlyphGeometry gg = m_MSDFData->Glyphs.at(0);
					//msdfgen::FontHandle* font = msdfgen::loadFontData();
				}


				if (destination - assetInfo.DataBuffer.Data == requiredSize)
				{
					GE_CORE_INFO("AssetSerializer::SerializeFontForPack() Successful.");
					return true;
				}
				else
				{
					GE_CORE_ASSERT(false, "Buffer overflow.");
				}
			}
			else
			{
				GE_CORE_ERROR("Required size is larger than given buffer size.");
				return 0;
			}
		}
		return false;
	}

	bool AssetSerializer::SerializeAudioForPack(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		/*
	   * - Handle
	   * - Type
	   * - Name
	   * ~ Config
	   * * - SourceID : not included
	   * * - Loop : bool
	   * * - Pitch : float
	   * * - Gain : float
	   * - Position : not included
	   * - Velocity : not included
	   * ~ Audio Buffer Data
	   * * - BPS : uint8_t
	   * * - Channels : uint8_t
	   * * - SampleRate : int32_t
	   * * - Format : int32_t
	   * * - Size : uint32_t
	   * * - Data : uint_8_t*
	   */

		Ref<AudioClip> audioClip = Project::GetAssetAs<AudioClip>(asset);
		assetInfo.Type = 4; // See Asset::Type
		uint64_t requiredSize = 0;

		// Size
		{
			requiredSize = Aligned(sizeof(uint64_t)) // Handle
				+ Aligned(sizeof(uint16_t))	// Type
				+ Aligned(sizeof(size_t)) // sizeof(stringLength)
				+ Aligned(audioClip->m_Config.Name.size() * sizeof(char)); //sizeof(string)

			// Config
			requiredSize += Aligned(sizeof(bool))       // Loop
				+ Aligned(sizeof(float))                // Pitch
				+ Aligned(sizeof(float));               // Gain

			// Audio Buffer Data
			requiredSize += Aligned(sizeof(uint8_t))                    // BPS
				+ Aligned(sizeof(uint8_t))                              // Channels
				+ Aligned(sizeof(int32_t))                              // SampleRate
				+ Aligned(sizeof(int32_t));                              // Format

			//if (&m_AudioBuffer->Data != nullptr)
			//{
			//    requiredSize += Aligned(sizeof(uint32_t))   // Size of data
			//        + this->m_AudioBuffer->Size;            // Data
			//}

		}

		// Allocate Size for Data
		assetInfo.InitializeData(requiredSize);

		// Data
		if (assetInfo.DataBuffer.Data)
		{
			if (assetInfo.DataBuffer.Size >= requiredSize)
			{

				// Start at beginning of buffer
				uint8_t* destination = assetInfo.DataBuffer.Data;
				uint8_t* end = destination + requiredSize;

				// Clear requiredSize from destination
				memset(destination, 0, requiredSize);

				// Fill out buffer
				{
					*(uint64_t*)destination = audioClip->p_Handle;
					destination += Aligned(sizeof(uint64_t));

					*(uint16_t*)destination = (uint16_t)audioClip->p_Type;
					destination += Aligned(sizeof(uint16_t));

					size_t stringSize = audioClip->m_Config.Name.size();
					*(size_t*)destination = stringSize;
					destination += Aligned(sizeof(stringSize));
					memcpy(destination, audioClip->m_Config.Name.c_str(), stringSize * sizeof(char));
					destination += Aligned(stringSize * sizeof(char));

					// Config
					*(bool*)destination = audioClip->m_Config.Loop;
					destination += Aligned(sizeof(bool));

					*(float*)destination = audioClip->m_Config.Pitch;
					destination += Aligned(sizeof(float));

					*(float*)destination = audioClip->m_Config.Gain;
					destination += Aligned(sizeof(float));

					// Audio Buffer Data

					*(uint8_t*)destination = audioClip->m_AudioBuffer->BPS;
					destination += Aligned(sizeof(uint8_t));

					*(uint8_t*)destination = audioClip->m_AudioBuffer->Channels;
					destination += Aligned(sizeof(uint8_t));

					*(uint32_t*)destination = audioClip->m_AudioBuffer->SampleRate;
					destination += Aligned(sizeof(uint32_t));

					*(uint32_t*)destination = audioClip->m_AudioBuffer->Format;
					destination += Aligned(sizeof(uint32_t));


					/*if (&m_AudioBuffer->Data != nullptr)
					{
						*(uint32_t*)destination = m_AudioBuffer->Size;
						destination += Aligned(sizeof(uint32_t));

						*(uint8_t*)destination = *m_AudioBuffer->Data;
						destination += this->m_AudioBuffer->Size;
					}*/

				}

				if (destination - assetInfo.DataBuffer.Data == requiredSize)
				{
					GE_CORE_INFO("AssetSerializer::SerializeAudioForPack() Successful.");
					return true;
				}
				else
				{
					GE_CORE_ASSERT(false, "Buffer overflow.");
				}
			}
			else
			{
				GE_CORE_ERROR("Required size is larger than given buffer size.");
				return 0;
			}
		}

		return false;
	}

}

		