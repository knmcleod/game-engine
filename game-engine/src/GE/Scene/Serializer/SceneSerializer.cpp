#include "GE/GEpch.h"

#include "SceneSerializer.h"

#include "GE/Core/UUID/UUID.h"

#include "GE/Scene/Components/Components.h"
#include "GE/Scene/Entity/Entity.h"

#include "GE/Scripting/Scripting.h"

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
	static std::string GetStringFromRigidBody2DType(Rigidbody2DComponent::BodyType type)
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
		GE_CORE_ASSERT(false, "Cannot convert Rigidbody2D BodyType to string.");
		return "";
	}

	static Rigidbody2DComponent::BodyType GetRigidBody2DTypeFromString(std::string typeString)
	{
		if (typeString == "Static") return Rigidbody2DComponent::BodyType::Static;
		if (typeString == "Dynamic") return Rigidbody2DComponent::BodyType::Dynamic;
		if (typeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

		GE_CORE_ASSERT(false, "Cannot convert string to Rigidbody2D BodyType. Set to default Static.");
		return Rigidbody2DComponent::BodyType::Static;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : m_Scene(scene)
	{

	}

	void SceneSerializer::SerializeText(const std::string& filePath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << m_Scene->GetName();
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity = { entityID, m_Scene.get() };
				if (!entity)
					return;
				SerializeEntity(out, entity);
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filePath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeBinary(const std::string& filePath)
	{
		GE_CORE_ASSERT(false, "Serialization of Binary Failed!");
	}

	bool SceneSerializer::DeserializeText(const std::string& filePath)
	{
		std::ifstream stream(filePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		GE_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuID = entity["Entity"].as<uint64_t>(); // Entity ID

				// TagComponent
				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				GE_CORE_TRACE("Deserializing entity with\n\tID = {0},\n\tName = {1}", uuID, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuID, name);

				// TransformComponent
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetOrAddComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				// CameraComponent
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.GetOrAddComponent<CameraComponent>();
					auto& cameraProps = cameraComponent["Camera"];

					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["Type"].as<int>());
					cc.Camera.SetCameraFOV(cameraProps["FOV"].as<float>());
					cc.Camera.SetNearClip(cameraProps["Near"].as<float>());
					cc.Camera.SetFarClip(cameraProps["Far"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				// SpriteRendererComponent
				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<SpriteRendererComponent>();
					src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
					if (spriteRendererComponent["Texture"])
					{
						src.Texture = Texture2D::Create(spriteRendererComponent["Texture"].as<std::string>());
					}

					if (spriteRendererComponent["TilingFactor"])
					{
						src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
					}
				}

				// CircleRendererComponent
				auto circleRendererComponent = entity["CircleRendererComponent"];
				if (circleRendererComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<CircleRendererComponent>();
					src.Color = circleRendererComponent["Color"].as<glm::vec4>();
					src.Radius = circleRendererComponent["Radius"].as<float>();
					src.Thickness = circleRendererComponent["Thickness"].as<float>();
					src.Fade = circleRendererComponent["Fade"].as<float>();
				}

				// NativeScriptComponent
				auto nativeScriptComponent = entity["NativeScriptComponent"];
				if (nativeScriptComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<NativeScriptComponent>();
				}

				// ScriptComponent
				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<ScriptComponent>();
					src.ClassName = scriptComponent["ClassName"].as<std::string>();

					// Fields
					auto fields = scriptComponent["ScriptFields"];
					if (fields)
					{
						Ref<ScriptClass> scriptClass = Scripting::GetScriptClass(src.ClassName);
						GE_CORE_ASSERT(scriptClass, "Script Class does not exist.");
						const auto& scriptClassFields = scriptClass->GetFields();
						auto& scriptFieldMap = Scripting::GetScriptFieldMap(deserializedEntity);

						for (auto field : fields)
						{
							std::string scriptFieldName = field["Name"].as<std::string>();
				
							if (scriptClassFields.find(scriptFieldName) == scriptClassFields.end())
							{
								GE_CORE_WARN("Cannot deserialize Entity Script Field. Script Field Name({0}) not found.", scriptFieldName);
								continue;
							}

							std::string scriptFieldTypeString = field["Type"].as<std::string>();
							ScriptFieldType scriptFieldType = StringToScriptFieldType(scriptFieldTypeString);
							
							ScriptFieldInstance& scriptFieldInstance = scriptFieldMap[scriptFieldName];
							scriptFieldInstance.Field = scriptClassFields.at(scriptFieldName);
							
							switch (scriptFieldType)
							{
								case GE::ScriptFieldType::None:
									break;
								case GE::ScriptFieldType::Char:
								{
									char data = field["Data"].as<char>();
									scriptFieldInstance.SetValue<char>(data);
									break;
								}
								case GE::ScriptFieldType::Int:
								{
									int32_t data = field["Data"].as<int32_t>();
									scriptFieldInstance.SetValue<int32_t>(data);
									break;
								}
								case GE::ScriptFieldType::UInt:
								{
									uint32_t data = field["Data"].as<uint32_t>();
									scriptFieldInstance.SetValue<uint32_t>(data);
									break;
								}
								case GE::ScriptFieldType::Float:
								{
									float data = field["Data"].as<float>();
									scriptFieldInstance.SetValue<float>(data);
									break;
								}
								case GE::ScriptFieldType::Byte:
								{
									int8_t data = field["Data"].as<int8_t>();
									scriptFieldInstance.SetValue<int8_t>(data);
									break;
								}
								case GE::ScriptFieldType::Bool:
								{
									bool data = field["Data"].as<bool>();
									scriptFieldInstance.SetValue<bool>(data);
									break;
								}
								case GE::ScriptFieldType::Vector2:
								{
									glm::vec2 data = field["Data"].as<glm::vec2>();
									scriptFieldInstance.SetValue<glm::vec2>(data);
									break;
								}
								case GE::ScriptFieldType::Vector3:
								{
									glm::vec3 data = field["Data"].as<glm::vec3>();
									scriptFieldInstance.SetValue<glm::vec3>(data);
									break;
								}
								case GE::ScriptFieldType::Vector4:
								{
									glm::vec4 data = field["Data"].as<glm::vec4>();
									scriptFieldInstance.SetValue<glm::vec4>(data);
									break;
								}
								case GE::ScriptFieldType::Entity:
								{
									UUID data = field["Data"].as<UUID>();
									scriptFieldInstance.SetValue<UUID>(data);
									break;
								}
							}
							
						}
					}
					
				}

				// RigidBody2DComponent
				auto rigidBody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidBody2DComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<Rigidbody2DComponent>();
					src.FixedRotation = rigidBody2DComponent["FixedRotation"].as<bool>();
					src.Type = GetRigidBody2DTypeFromString(rigidBody2DComponent["Type"].as<std::string>());
				}

				// BoxCollider2DComponent
				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<BoxCollider2DComponent>();
					src.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					src.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
					src.Density = boxCollider2DComponent["Density"].as<float>();
					src.Friction = boxCollider2DComponent["Friction"].as<float>();
					src.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					src.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();

					src.Show = boxCollider2DComponent["Show"].as<bool>();
				}

				// CircleCollider2DComponent
				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& src = deserializedEntity.GetOrAddComponent<CircleCollider2DComponent>();
					src.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
					src.Radius = circleCollider2DComponent["Radius"].as<float>();
					src.Density = circleCollider2DComponent["Density"].as<float>();
					src.Friction = circleCollider2DComponent["Friction"].as<float>();
					src.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					src.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();

					src.Show = circleCollider2DComponent["Show"].as<bool>();
				}
			}
		}
		
		return true;
	}

	bool SceneSerializer::DeserializeBinary(const std::string& filePath)
	{
		GE_CORE_ASSERT(false, "Deserialization of Binary Failed!");
		return false;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		GE_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Cannot serialize Entity without ID.");

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent
			auto& tag = entity.GetComponent<TagComponent>().Tag;

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
			auto& camera = component.Camera;

			out << YAML::Key << "FixedAspectRatio" << YAML::Value << component.FixedAspectRatio;
			out << YAML::Key << "Primary" << YAML::Value << component.Primary;
			
			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera

			out << YAML::Key << "Far" << YAML::Value << camera.GetFarClip();
			out << YAML::Key << "Near" << YAML::Value << camera.GetNearClip();
			out << YAML::Key << "FOV" << YAML::Value << camera.GetCameraFOV();
			out << YAML::Key << "Type" << YAML::Value << (int)camera.GetProjectionType();

			out << YAML::EndMap; // Camera
			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent
			auto& component = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << component.Color;
			if (component.Texture)
			{
				out << YAML::Key << "Texture" << YAML::Value << component.Texture->GetPath();
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

						out << YAML::BeginMap; // ScriptFields

						out << YAML::Key << "Name" << YAML::Value << name;
						out << YAML::Key << "Type" << YAML::Value << ScriptFieldTypeToString(field.Type);
						out << YAML::Key << "Data" << YAML::Value;

						ScriptFieldInstance& scriptFieldInstance = scriptFieldMap.at(name);
						switch (field.Type)
						{
						case ScriptFieldType::Char:
							out << scriptFieldInstance.GetValue<char>();
							break;
						case ScriptFieldType::Int:
							out << scriptFieldInstance.GetValue<int>();
							break;
						case ScriptFieldType::UInt:
							out << scriptFieldInstance.GetValue<uint32_t>();
							break;
						case ScriptFieldType::Float:
							out << scriptFieldInstance.GetValue<float>();
							break;
						case ScriptFieldType::Byte:
							out << scriptFieldInstance.GetValue<int8_t>();
							break;
						case ScriptFieldType::Bool:
							out << scriptFieldInstance.GetValue<bool>();
							break;
						case ScriptFieldType::Vector2:
							out << scriptFieldInstance.GetValue<glm::vec2>();
							break;
						case ScriptFieldType::Vector3:
							out << scriptFieldInstance.GetValue<glm::vec3>();
							break;
						case ScriptFieldType::Vector4:
							out << scriptFieldInstance.GetValue<glm::vec4>();
							break;
						case ScriptFieldType::Entity:
							out << scriptFieldInstance.GetValue<UUID>();
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
			out << YAML::Key << "Type" << YAML::Value << GetStringFromRigidBody2DType(component.Type);
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
}
