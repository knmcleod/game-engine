#include "GE/GEpch.h"
#include "SceneSerializer.h"
#include "GE/Scene/Components/Components.h"

#include <yaml-cpp/yaml.h>

namespace YAML {

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

}

namespace GE
{
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

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : m_Scene(scene)
	{

	}

	void SceneSerializer::SerializeText(const std::string& filePath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Name";
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

				Entity deserializedEntity = m_Scene->CreateEntity(name);

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
				}

				// NativeScriptComponent
				
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
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << "12837192831273"; // Entity ID

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
			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{

		}

		out << YAML::EndMap; // Entity
	}
}
