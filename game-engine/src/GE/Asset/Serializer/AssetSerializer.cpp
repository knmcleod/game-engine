#include "GE/GEpch.h"

#include "AssetSerializer.h"

#include "GE/Audio/AudioManager.h"
#include "GE/Asset/Assets/Scene/Scene.h"

#include "GE/Asset/RuntimeAssetManager.h"

#include "GE/Project/Project.h"

#include "GE/Scripting/Scripting.h"

#include <al.h>
#include <stb_image.h>
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
	std::map<Asset::Type, AssetDeserializeFunction> AssetSerializer::s_AssetDeserializeFuncs =
	{
		{ Asset::Type::Scene,		AssetSerializer::DeserializeScene		},
		{ Asset::Type::Texture2D,	AssetSerializer::DeserializeTexture2D	},
		{ Asset::Type::Font,		AssetSerializer::DeserializeFont		},
		{ Asset::Type::Audio,		AssetSerializer::DeserializeAudio		},
	};

	std::map<Asset::Type, AssetPackDeserializeFunction> AssetSerializer::s_AssetPackDeserializeFuncs =
	{
		{ Asset::Type::Scene,		AssetSerializer::DeserializeSceneFromPack		},
		{ Asset::Type::Texture2D,	AssetSerializer::DeserializeTexture2DFromPack	},
		{ Asset::Type::Font,		AssetSerializer::DeserializeFontFromPack		},
		{ Asset::Type::Audio,		AssetSerializer::DeserializeAudioFromPack		},
	};

	std::map<Asset::Type, AssetSerializeFunction> AssetSerializer::s_AssetSerializeFuncs =
	{
		{ Asset::Type::Scene, AssetSerializer::SerializeScene }
	};

	std::map<Asset::Type, AssetPackSerializeFunction> AssetSerializer::s_AssetPackSerializeFuncs =
	{
		{ Asset::Type::Scene,		AssetSerializer::SerializeSceneForPack		},
		{ Asset::Type::Texture2D,	AssetSerializer::SerializeTexture2DForPack	},
		{ Asset::Type::Font,		AssetSerializer::SerializeFontForPack		},
		{ Asset::Type::Audio,		AssetSerializer::SerializeAudioForPack		}
	};

#pragma region Entity

	static void SerializeEntityByString(YAML::Emitter& out, const Entity& e)
	{
		GE_CORE_ASSERT(e.HasComponent<IDComponent>(), "Cannot serialize Entity without ID.");

		UUID eHandle = e.GetComponent<IDComponent>().ID;
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << eHandle;

		if (e.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent
			auto& tag = e.GetComponent<TagComponent>().Tag;
			GE_CORE_TRACE("Entity\n\tUUID : {0},\n\tName : {1}", (uint64_t)eHandle, tag.c_str());
			
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (e.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent
			auto& component = e.GetComponent<TransformComponent>();

			out << YAML::Key << "Translation" << YAML::Value << component.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << component.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << component.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (e.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent
			auto& component = e.GetComponent<CameraComponent>();
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

		if (e.HasComponent<AudioSourceComponent>())
		{
			out << YAML::Key << "AudioSourceComponent";
			out << YAML::BeginMap; // AudioSourceComponent
			auto& component = e.GetComponent<AudioSourceComponent>();

			out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;
			out << YAML::Key << "Gain" << YAML::Value << component.Gain;
			out << YAML::Key << "Pitch" << YAML::Value << component.Pitch;
			out << YAML::Key << "Loop" << YAML::Value << component.Loop;

			out << YAML::EndMap; // AudioSourceComponent
		}

		if (e.HasComponent<AudioListenerComponent>())
		{
			out << YAML::Key << "AudioListenerComponent";
			out << YAML::BeginMap; // AudioListenerComponent
			auto& component = e.GetComponent<AudioListenerComponent>();

			out << YAML::EndMap; // AudioListenerComponent
		}

		if (e.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent
			auto& component = e.GetComponent<SpriteRendererComponent>();
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

		if (e.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent
			auto& component = e.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << component.Color;
			out << YAML::Key << "Radius" << YAML::Value << component.Radius;
			out << YAML::Key << "Thickness" << YAML::Value << component.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << component.Fade;
			out << YAML::EndMap; // CircleRendererComponent
		}

		if (e.HasComponent<TextRendererComponent>())
		{
			out << YAML::Key << "TextRendererComponent";
			out << YAML::BeginMap; // TextRendererComponent
			auto& component = e.GetComponent<TextRendererComponent>();
			out << YAML::Key << "TextColor" << YAML::Value << component.TextColor;
			out << YAML::Key << "BGColor" << YAML::Value << component.BGColor;

			out << YAML::Key << "KerningOffset" << YAML::Value << component.KerningOffset;
			out << YAML::Key << "LineHeightOffset" << YAML::Value << component.LineHeightOffset;
			out << YAML::Key << "Text" << YAML::Value << component.Text;

			out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;
			out << YAML::EndMap; // TextRendererComponent
		}

		if (e.HasComponent<NativeScriptComponent>())
		{
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap; // NativeScriptComponent
			auto& component = e.GetComponent<NativeScriptComponent>();

			out << YAML::EndMap; // NativeScriptComponent
		}

		if (e.HasComponent<ScriptComponent>())
		{
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent
			auto& component = e.GetComponent<ScriptComponent>();

			// TODO : Move everything below to SerializeAsset(Ref<Asset>, AssetMetadata&) for Script Asset
			// Replace with Script Asset UUID

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

					auto& scriptFieldMap = Scripting::GetScriptFieldMap(e);
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

		if (e.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent
			auto& component = e.GetComponent<Rigidbody2DComponent>();
			const std::string typeString = ComponentUtils::GetStringFromRigidBody2DType(component.Type);
			out << YAML::Key << "Type" << YAML::Value << typeString;
			out << YAML::Key << "FixedRotation" << YAML::Value << component.FixedRotation;
			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (e.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent
			auto& component = e.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "Size" << YAML::Value << component.Size;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;
			out << YAML::Key << "Show" << YAML::Value << component.Show;
			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (e.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent
			auto& component = e.GetComponent<CircleCollider2DComponent>();
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

	static bool DeserializeEntityFromString(const YAML::detail::iterator_value& eDetails, Entity& e)
	{
		// TransformComponent
		auto transformComponent = eDetails["TransformComponent"];
		if (transformComponent)
		{
			auto& tc = e.GetOrAddComponent<TransformComponent>();
			tc.Translation = transformComponent["Translation"].as<glm::vec3>();
			tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
			tc.Scale = transformComponent["Scale"].as<glm::vec3>();
		}

		// CameraComponent
		auto cameraComponent = eDetails["CameraComponent"];
		if (cameraComponent)
		{
			auto& cc = e.GetOrAddComponent<CameraComponent>();
			auto& cameraProps = cameraComponent["Camera"];

			cc.ActiveCamera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["Type"].as<int>());
			cc.ActiveCamera.SetInfo(cameraProps["FOV"].as<float>(), cameraProps["Near"].as<float>(), cameraProps["Far"].as<float>());

			cc.Primary = cameraComponent["Primary"].as<bool>();
			cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
		}

		// AudioSourceComponent
		auto audioSourceComponent = eDetails["AudioSourceComponent"];
		if (audioSourceComponent)
		{
			auto& cc = e.GetOrAddComponent<AudioSourceComponent>();
			cc.AssetHandle = audioSourceComponent["AssetHandle"].as<UUID>();
			cc.Gain = audioSourceComponent["Gain"].as<float>();
			cc.Pitch = audioSourceComponent["Pitch"].as<float>();
			cc.Loop = audioSourceComponent["Loop"].as<bool>();
		}

		// AudioListenerComponent
		auto audioListenerComponent = eDetails["AudioListenerComponent"];
		if (audioListenerComponent)
		{
			auto& cc = e.GetOrAddComponent<AudioListenerComponent>();

		}

		// SpriteRendererComponent
		auto spriteRendererComponent = eDetails["SpriteRendererComponent"];
		if (spriteRendererComponent)
		{
			auto& src = e.GetOrAddComponent<SpriteRendererComponent>();
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
			auto& src = e.GetOrAddComponent<CircleRendererComponent>();
			src.Color = circleRendererComponent["Color"].as<glm::vec4>();
			src.Radius = circleRendererComponent["Radius"].as<float>();
			src.Thickness = circleRendererComponent["Thickness"].as<float>();
			src.Fade = circleRendererComponent["Fade"].as<float>();
		}

		// TextRendererComponent
		auto textRendererComponent = eDetails["TextRendererComponent"];
		if (textRendererComponent)
		{
			auto& src = e.GetOrAddComponent<TextRendererComponent>();
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
			auto& src = e.GetOrAddComponent<NativeScriptComponent>();
		}

		// ScriptComponent
		auto scriptComponent = eDetails["ScriptComponent"];
		if (scriptComponent)
		{
			Scripting::SetScene(e.GetScene());

			auto& src = e.GetOrAddComponent<ScriptComponent>();
			
			// TODO : Move everything below to DeserializeAsset(AssetMetadata&) for Script Asset
			// Replace with Script Asset UUID

			src.ClassName = scriptComponent["ClassName"].as<std::string>();

			// Fields
			auto fields = scriptComponent["ScriptFields"];
			if (fields)
			{
				Ref<ScriptClass> scriptClass = Scripting::GetScriptClass(src.ClassName);
				GE_CORE_ASSERT(scriptClass, "Script Class does not exist.");
				const auto& scriptClassFields = scriptClass->GetFields();
				auto& scriptFieldMap = Scripting::GetScriptFieldMap(e);

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
			auto& src = e.GetOrAddComponent<Rigidbody2DComponent>();
			src.FixedRotation = rigidBody2DComponent["FixedRotation"].as<bool>();
			src.Type = ComponentUtils::GetRigidBody2DTypeFromString(rigidBody2DComponent["Type"].as<std::string>());
		}

		// BoxCollider2DComponent
		auto boxCollider2DComponent = eDetails["BoxCollider2DComponent"];
		if (boxCollider2DComponent)
		{
			auto& src = e.GetOrAddComponent<BoxCollider2DComponent>();
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
			auto& src = e.GetOrAddComponent<CircleCollider2DComponent>();
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

#pragma endregion

#pragma region Texture2D
	/*
	* Expects full filePath
	*/
	static Buffer LoadTextureDataFromFile(const std::string& filePath, int& width, int& height, int& channels)
	{
		GE_PROFILE_SCOPE("stbi_load - LoadTextureDataFromFile()");
		stbi_set_flip_vertically_on_load(1);
		// Size assumed 1 byte per channel
		uint8_t* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
		GE_CORE_ASSERT(data, "Failed to load stb_image!");
		Buffer dataBuffer = Buffer(data, width * height * channels);
		delete[] data;
		return dataBuffer;
	}

#pragma endregion

#pragma region Font

	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> func>
	static Ref<Texture2D> LoadFontAtlas(const std::filesystem::path& filePath, Font::AtlasConfig& atlasConfig, Ref<Font::MSDFData> msdfData)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
		{
			GE_CORE_ERROR("Failed to load Font Freetype Handle");
			return 0;
		}

		std::filesystem::path path = Project::GetPathToAsset(filePath);
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, path.string().c_str());
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
		Texture::Config config;
		config.Height = bitmap.height;
		config.Width = bitmap.width;
		config.InternalFormat = Texture::ImageFormat::RGB8;
		config.Format = Texture::DataFormat::RGB;
		config.GenerateMips = false;
	
		Buffer dataBuffer((void*)bitmap.pixels, 
			bitmap.height * bitmap.width * (config.InternalFormat == Texture::ImageFormat::RGB8 ? 3 : 4));
		Ref<Texture2D> texture = Texture2D::Create(config, dataBuffer);
		dataBuffer.Release();

		return texture;
	}
	
	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> func>
	static Ref<Texture2D> LoadFontAtlas(Buffer data, const uint32_t& width, const uint32_t& height, Font::AtlasConfig& atlasConfig, Ref<Font::MSDFData> msdfData)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
		{
			GE_CORE_ERROR("Failed to load Font Freetype Handle");
			return 0;
		}

		msdfgen::FontHandle* font = msdfgen::loadFontData(ft, data.As<uint8_t>(), data.GetSize());
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

		Texture::Config config;
		config.Height = height;
		config.Width = width;
		config.InternalFormat = Texture::ImageFormat::RGB8;
		config.Format = Texture::DataFormat::RGB;
		config.GenerateMips = false;

		Buffer dataBuffer(data.As<void>(),
			config.Height* config.Width * (config.InternalFormat == Texture::ImageFormat::RGB8 ? 3 : 4));
		Ref<Texture2D> texture = Texture2D::Create(config, dataBuffer);
		dataBuffer.Release();

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

#pragma endregion

	bool AssetSerializer::DeserializeRegistry(Ref<AssetRegistry> registry)
	{
		const std::filesystem::path& path = Project::GetPathToAsset(registry->m_FilePath);
		if (path.empty())
			return false;
		GE_CORE_TRACE("Asset Registry Deserialization Started.\n\tFilePath : {0}", path.string().c_str());

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
			GE_CORE_WARN("Asset Registry Deserialization Failed.\n\tAssetRegistry Node not found.");
			return false;
		}

		for (const auto& node : assetRegistryData)
		{
			UUID handle = node["Handle"].as<uint64_t>();
			AssetMetadata assetMetadata;
			assetMetadata.Handle = handle;
			assetMetadata.FilePath = node["FilePath"].as<std::string>();
			assetMetadata.Type = AssetUtils::AssetTypeFromString(node["Type"].as<std::string>());
			GE_CORE_TRACE("Asset\n\tUUID : {0}\n\tFilePath : {1}\n\tType : {2}", (uint64_t)assetMetadata.Handle, assetMetadata.FilePath.string().c_str(), AssetUtils::AssetTypeToString(assetMetadata.Type).c_str());
			if (!registry->AddAsset(assetMetadata))
				GE_CORE_WARN("Failed to add Asset::{0} : {1}\n\tFilePath : {2}", AssetUtils::AssetTypeToString(assetMetadata.Type).c_str(), (uint64_t)assetMetadata.Handle, assetMetadata.FilePath.string().c_str());
		}
		GE_CORE_INFO("Asset Registry Deserialization Complete.");
		return true;
	}

	bool AssetSerializer::DeserializePack(Ref<AssetPack> pack)
	{
		// .gap(Game Asset Pack) file
		//  [bytes]

		//	[8] header, info
		//  [4] signature // File Extension "GAP"
		//  [4] Version // File Format Version

		//	[100 + ?] index, data. Size depends on Assets & Entities.
		//  [8] Size	// Size of all Scenes, Count & Map
		//	[8] Scene Map Count
		//  [84 + ?] Scene Map			// Size based on how many Scenes are loaded
		//      Asset Handle    : Key
		//      [84 + ?] SceneInfo  : Value
		//          [8] Packed Size : Size of whole Scene
		//			[76 + ?] Data
		//				[8] Handle
		//				[2] Type
		//				[8] Name
		//				[8] Asset Map Count
		//				[26 + ?] Asset Map			// Size based on how many Assets are loaded
		//				    Asset Handle    : Key
		//				    [26 + ?] AssetInfo	: Value
		//				        [8] Packed Size
		//						[18 + ?] Packed Data
		//							[8] Handle
		//							[2] Type
		//							[8] Name
		//							[?] Asset Specific Info : See DeserializeAsset(AssetInfo&)
		//				[8] Entity Map Count
		//				[16 + ?] Entity Map			// Size based on how many Entities are loaded
		//				    Handle			: Key
		//				    [8 + ?] EntityInfo	: Value
		//				        [8] Packed Size
		//						[?]	Packed Data
		//							[?] Entity Specific Info : See DeserializeEntity(const EntityInfo&, Entity&)

		std::filesystem::path path = Project::GetPathToAsset(pack->m_File.Path);
		GE_CORE_INFO("Asset Pack Deserialization Started.\n\tFilePath : {0}", path.string().c_str());

		std::ifstream stream(path, std::ios::app | std::ios::binary);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Pack file to read.");
			return false;
		}

		// Header
		{
			stream.read((char*)&pack->m_File.FileHeader.HEADER, sizeof(pack->m_File.FileHeader.HEADER));
			stream.read((char*)&pack->m_File.FileHeader.Version, sizeof(pack->m_File.FileHeader.Version));
		}

		// Index
		{
			stream.read((char*)&pack->m_File.Index.Size, sizeof(pack->m_File.Index.Size));
			uint64_t sceneCount = 0;
			stream.read((char*)&sceneCount, sizeof(sceneCount));
			for (uint64_t i = 0; i < sceneCount; i++)
			{
				SceneInfo sceneInfo = SceneInfo();

				size_t size = 0;
				stream.read((char*)&size, sizeof(size));

				uint8_t* data = new uint8_t[size];
				stream.read((char*)data, size * sizeof(uint8_t));

				sceneInfo.DataBuffer = Buffer(data, size);

				delete[] data;

				if (Ref<Asset> sceneAsset = DeserializeAsset(sceneInfo))
				{
					if (Project::GetAssetManager()->AddAsset(sceneAsset) && pack->AddAsset(sceneAsset, sceneInfo))
					{
						for (const auto& [uuid, childAssetInfo] : sceneInfo.m_Assets)
						{
							if (Ref<Asset> childAsset = DeserializeAsset(childAssetInfo))
							{
								Project::GetAssetManager()->AddAsset(childAsset);
							}
						}

						Ref<Scene> scene = Project::GetAssetAs<Scene>(sceneAsset);
						for (const auto& [uuid, entityInfo] : sceneInfo.m_Entities)
						{
							// Entity is created in DeserializeAsset<Scene>(AssetInfo&)
							Entity e = scene->GetEntityByUUID(uuid);
							if (!DeserializeEntity(entityInfo, e))
							{
								scene->DestroyEntity(e);
							}
						}
					}
				}
				else
				{
					GE_CORE_ERROR("Could not deserialize Scene Asset.");
				}
				sceneInfo.ClearAllData();
			}
		}

		if (stream.is_open() && stream.good())
		{
			stream.close();
			return true;
		}
		return false;
	}

	bool AssetSerializer::SerializeRegistry(Ref<AssetRegistry> registry)
	{
		// .gar(Game Asset Registry) file

		std::filesystem::path path = Project::GetPathToAsset(registry->m_FilePath);
		GE_CORE_INFO("AssetRegistry Serialization Started.\n\tFilePath : {0}", path.string().c_str());

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;
			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : registry->GetRegistry())
			{
				GE_CORE_TRACE("Asset\n\tUUID : {0}\n\tFilePath : {1}\n\tType : {2}", (uint64_t)handle, metadata.FilePath.string().c_str(), AssetUtils::AssetTypeToString(metadata.Type).c_str());

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
			GE_CORE_INFO("Asset Registry Serialization Complete.");
			return true;
		}

		GE_CORE_WARN("Asset Registry Serialization Failed.");
		return false;
	}

	bool AssetSerializer::SerializePack(Ref<AssetPack> pack)
	{
		// .gap(Game Asset Pack) file
		//  [bytes]
		
		//	[8] header, info
		//  [4] signature // File Extension "GAP"
		//  [4] Version // File Format Version
		
		//	[100 + ?] index, data. Size depends on Assets & Entities.
		//  [8] Size	// Size of all Scenes, Count & Map
		//	[8] Scene Map Count
		//  [84 + ?] Scene Map			// Size based on how many Scenes are loaded
		//      Asset Handle    : Key
		//      [84 + ?] SceneInfo  : Value
		//          [8] Packed Size : Size of whole Scene
		//			[76 + ?] Data
		//				[8] Handle
		//				[2] Type
		//				[8] Name
		//				[8] Asset Map Count
		//				[26 + ?] Asset Map			// Size based on how many Assets are loaded
		//				    Asset Handle    : Key
		//				    [26 + ?] AssetInfo	: Value
		//				        [8] Packed Size
		//						[18 + ?] Packed Data
		//							[8] Handle
		//							[2] Type
		//							[8] Name
		//							[?] Asset Specific Info : See DeserializeAsset(AssetInfo&)
		//				[8] Entity Map Count
		//				[16 + ?] Entity Map			// Size based on how many Entities are loaded
		//				    Handle			: Key
		//				    [8 + ?] EntityInfo	: Value
		//				        [8] Packed Size
		//						[?]	Packed Data
		//							[?] Entity Specific Info : See DeserializeEntity(const EntityInfo&, Entity&)

		std::filesystem::path path = Project::GetPathToAsset(pack->m_File.Path);
		GE_CORE_INFO("AssetRegistry Serialization Started.\n\tFilePath : {0}", path.string().c_str());

		std::ofstream stream(path, std::ios::app | std::ios::binary);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Pack file to write.");
			return false;
		}

		// Header
		{
			stream.write(reinterpret_cast<const char*>(pack->m_File.FileHeader.HEADER), sizeof(pack->m_File.FileHeader.HEADER));
			stream.write(reinterpret_cast<const char*>(&pack->m_File.FileHeader.Version), sizeof(pack->m_File.FileHeader.Version));
		}
			
		// Index
		{
			for (const auto& [uuid, asset] : Project::GetAssetManager<RuntimeAssetManager>()->GetLoadedAssets())
			{
				if (asset->GetType() == Asset::Type::Scene)
				{
					if (SerializeAsset(asset, pack->m_File.Index.Scenes[uuid]))
					{
						// Scene and its children have been populated into pack->m_File.Index.Scenes.at(uuid)
						// Scene.Size should represent the total scene size and be aligned
						// Add to Index.Size for each Scene Asset
						pack->m_File.Index.Size += pack->m_File.Index.Scenes.at(uuid).DataBuffer.GetSize();
					}
				}
			}

			// Index.Size contains
			// - Scene Count
			// - All Scenes
			pack->m_File.Index.Size += GetAligned(sizeof(pack->m_File.Index.Scenes.size()));
			stream.write(reinterpret_cast<const char*>(&pack->m_File.Index.Size), sizeof(pack->m_File.Index.Size));

			uint64_t allScenesCount = pack->m_File.Index.Scenes.size();
			stream.write(reinterpret_cast<const char*>(&allScenesCount), sizeof(uint64_t));
			for (const auto& [sceneHandle, sceneInfo] : pack->m_File.Index.Scenes)
			{
				// Write all data
				// SceneInfo.Data contains all AssetInfo & EntityInfo
				size_t sceneSize = sceneInfo.DataBuffer.GetSize();
				stream.write(reinterpret_cast<const char*>(&sceneSize), sizeof(sceneSize));
				stream.write(reinterpret_cast<const char*>(sceneInfo.DataBuffer.As<char>()), sceneSize * sizeof(uint8_t));

			}
		}
		
		if (stream.is_open() && stream.good())
		{
			stream.close();
			GE_CORE_TRACE("Asset Pack Serialization Complete");
			return true;
		}
		GE_CORE_WARN("Asset Pack Serialization Failed.");
		return false;
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

#pragma region Metadata

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
		scene->m_Config.Name = sceneName;
		GE_CORE_TRACE("Deserializing Scene\n\tUUID : {0}\n\tName : {1}\n\tPath : {2}", (uint64_t)metadata.Handle, scene->GetName().c_str(), path.string());

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto e : entities)
			{
				// ID Component 
				uint64_t uuid = e["Entity"].as<uint64_t>(); // UUID

				// TagComponent
				std::string name = std::string();
				auto tagComponent = e["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				GE_CORE_TRACE("Entity\n\tUUID : {0},\n\tName : {1}", uuid, name);

				Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name);
				DeserializeEntityFromString(e, deserializedEntity);
			}
		}
		
		scene->p_Status = Asset::Status::Ready;
		return scene;
	}

	Ref<Asset> AssetSerializer::DeserializeTexture2D(const AssetMetadata& metadata)
	{
		int width = 0, height = 0, channels = 4;
		stbi_set_flip_vertically_on_load(1);
		Buffer data = LoadTextureDataFromFile(Project::GetPathToAsset(metadata.FilePath).string(), width, height, channels);

		Texture::Config config;
		config.Name = metadata.FilePath.filename().string();
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
		{
			GE_CORE_WARN("Unsupported Texture2D Channels.");
			return nullptr;
		}

		Ref<Texture2D> texture = Texture2D::Create(config, data);
		data.Release();
		if (texture)
		{
			texture->p_Handle = metadata.Handle;
		}
		return texture;
	}

	Ref<Asset> AssetSerializer::DeserializeFont(const AssetMetadata& metadata)
	{
		Ref<Font> asset = CreateRef<Font>(metadata.Handle);
		asset->p_Status = Asset::Status::Loading;
		asset->m_AtlasConfig.Texture =  LoadFontAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>
			(metadata.FilePath, asset->m_AtlasConfig, asset->m_MSDFData);

		asset->p_Status = Asset::Status::Ready;
		return asset;
	}

	Ref<Asset> AssetSerializer::DeserializeAudio(const AssetMetadata& metadata)
	{	
		// Read data from filePath
		{
			const std::filesystem::path filePath = Project::GetPathToAsset(metadata.FilePath);
			std::ifstream stream(filePath, std::ios::binary);
			if (!stream.is_open())
			{
				GE_CORE_ERROR("Could not open WAV file {0}", filePath.string().c_str());
				return nullptr;
			}

			{
				char data[4];

				// the RIFF
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read RIFF while loading Wav file.");
					return false;
				}
				if (std::strncmp(data, "RIFF", 4) != 0)
				{
					GE_CORE_ERROR("File is not a valid WAVE file (header doesn't begin with RIFF)");
					return false;
				}

				// the size of the file
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read size of Wav file.");
					return false;
				}

				// the WAVE
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read WAVE");
					return false;
				}
				if (std::strncmp(data, "WAVE", 4) != 0)
				{
					GE_CORE_ERROR("File is not a valid WAVE file (header doesn't contain WAVE)");
					return false;
				}

				// "fmt/0"
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read fmt of Wav file.");
					return false;
				}

				// this is always 16, the size of the fmt data chunk
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read the size of the fmt data chunk. Should be 16.");
					return false;
				}

				// PCM should be 1?
				if (!stream.read(data, 2))
				{
					GE_CORE_ERROR("Could not read PCM. Should be 1.");
					return false;
				}

				// the number of Channels
				if (!stream.read(data, 2))
				{
					GE_CORE_ERROR("Could not read number of Channels.");
					return false;
				}
				uint32_t channels = convert_to_int(data, 2);

				// sample rate
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read sample rate.");
					return false;
				}
				uint32_t sampleRate = convert_to_int(data, 4);

				// (SampleRate * BPS * Channels) / 8
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read (SampleRate * BPS * Channels) / 8");
					return false;
				}

				// ?? dafaq
				if (!stream.read(data, 2))
				{
					GE_CORE_ERROR("Could not read dafaq?");
					return false;
				}

				// BPS
				if (!stream.read(data, 2))
				{
					GE_CORE_ERROR("Could not read bits per sample.");
					return false;
				}
				uint32_t bps = convert_to_int(data, 2);

				// data chunk header "data"
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read data chunk header.");
					return false;
				}
				if (std::strncmp(data, "data", 4) != 0)
				{
					GE_CORE_ERROR("File is not a valid WAVE file (doesn't have 'data' tag).");
					return false;
				}

				// size of data
				if (!stream.read(data, 4))
				{
					GE_CORE_ERROR("Could not read data size.");
					return false;
				}
				size_t size = convert_to_int(data, 4);

				/* cannot be at the end of file */
				if (stream.eof())
				{
					GE_CORE_ERROR("Reached EOF.");
					return false;
				}

				uint32_t bufferCount = size > AudioManager::BUFFER_SIZE ? (size / AudioManager::BUFFER_SIZE) + 1 : 1;
				Buffer buffer = Buffer(AudioManager::BUFFER_SIZE * bufferCount);
				// Read Data
				if (!stream.read(buffer.As<char>(), size))
				{
					GE_CORE_ERROR("Could not read data.");
					return false;
				}

				if (stream.fail())
				{
					GE_CORE_ERROR("File stream Failed.");
					return false;
				}
				stream.close();

				Ref<Audio> audio = Audio::Create(metadata.Handle, Audio::Config(metadata.FilePath.filename().string(), channels, sampleRate, bps, buffer), bufferCount);
				buffer.Release();

				return audio;
			}

		}
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

			scene->m_Registry.each([&](auto eID)
				{
					Entity e(eID, scene.get());
					SerializeEntityByString(out, e);
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

#pragma endregion

#pragma region AssetInfo

	Ref<Asset> AssetSerializer::DeserializeSceneFromPack(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Scene from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		// ScenePackFormat
		//      [84 + ?] SceneInfo  : Value, corresponding Key handled in SerializePack
		//          [8] Packed Size : Size of whole Scene
		//			[76 + ?] Data
		//				[8] Handle
		//				[2] Type
		//				[8] Name
		//				[8] Asset Map Count
		//				[26 + ?] Asset Map			// Size based on how many Assets are loaded
		//				    Asset Handle    : Key
		//				    [26 + ?] AssetInfo	: Value
		//				        [8] Packed Size
		//						[18 + ?] Packed Data
		//							[8] Handle
		//							[2] Type
		//							[8] Name
		//							[?] Asset Specific Info : See DeserializeAsset(AssetInfo&)
		//				[8] Entity Map Count
		//				[16 + ?] Entity Map			// Size based on how many Entities are loaded
		//				    Handle			: Key
		//				    [8 + ?] EntityInfo	: Value
		//				        [8] Packed Size
		//						[?]	Packed Data
		//							[?] Entity Specific Info : See DeserializeEntity(const EntityInfo&, Entity&)

		uint64_t handle = 0;
		Scene::Config config;

		// Read Data & Assign
		const uint8_t* source = assetInfo.DataBuffer.As<uint8_t>();
		const uint8_t* end = source + assetInfo.DataBuffer.GetSize();

		// Handle
		if (!ReadAligned(source, end, handle))
			return nullptr;

		// Type: Already read, skip
		source += GetAligned(sizeof(uint16_t));

		//Name
		uint64_t sizeofString = 0;
		if (!ReadAligned(source, end, sizeofString))
			return nullptr;
		char* name = new char[sizeofString];
		if (!ReadAlignedArray<char>(source, end, name, sizeofString))
			return nullptr;
		config.Name.assign(name, sizeofString);
		delete[](name);

		// Frame Steps
		if (!ReadAligned(source, end, config.StepFrames))
			return nullptr;

		Ref<Scene> scene = CreateRef<Scene>(handle, config);

		// Assets
		uint64_t assetCount = 0;
		ReadAligned(source, end, assetCount);
		for (int i = 0; i < assetCount; i++)
		{
			// Get Data from source. Will put source pointer at the end of assetData[i], don't use source again till next iteration
			uint64_t size = 0;
			// Read size of array first
			if (!ReadAligned(source, end, size))
				return nullptr;
			uint8_t* data = new uint8_t[size];
			if (!ReadAlignedArray<uint8_t>(source, end, data, size))
				return nullptr;

			Buffer cadBuffer = Buffer(data, size);
			delete[] data;

			// Overflow check
			if (source > end)
				GE_CORE_ASSERT(false, "AssetSerializer::DeserializeSceneFromPack(AssetInfo&) Buffer Overflow");

			// Set Data
			{
				const uint8_t* childSource = cadBuffer.As<uint8_t>();
				const uint8_t* endCAD = childSource + cadBuffer.GetSize();

				// UUID
				uint64_t uuid = 0;
				if (!ReadAligned(childSource, endCAD, uuid))
					return nullptr;

				// Type
				uint16_t type = 0;
				if (!ReadAligned(childSource, endCAD, type))
					return nullptr;

				((SceneInfo&)assetInfo).m_Assets[uuid].Type = type;
				((SceneInfo&)assetInfo).m_Assets.at(uuid).InitializeData(cadBuffer.GetSize(), cadBuffer.As<uint8_t>());

			}

			cadBuffer.Release();
		}

		// Entities
		uint64_t eCount = 0;
		ReadAligned(source, end, eCount);
		for (int i = 0; i < eCount; i++)
		{
			// Get
			uint64_t size = 0;
			// Read size of array first
			if (!ReadAligned(source, end, size))
				return nullptr;
			uint8_t* data = new uint8_t[size];
			if(!ReadAlignedArray<uint8_t>(source, end, data, size))
				return nullptr;

			Buffer eDataBuffer = Buffer(data, size);
			delete[] data;

			// Overflow check
			if (source > end)
				GE_CORE_ASSERT(false, "AssetSerializer::DeserializeSceneFromPack(AssetInfo&) Buffer Overflow");

			// Set
			{
				const uint8_t* entitySource = eDataBuffer.As<uint8_t>();
				const uint8_t* endED = entitySource + eDataBuffer.GetSize();

				// Component Type : IDComponent(1) expected
				uint16_t cType = 0;
				if (!ReadAligned(entitySource, endED, cType))
					return nullptr;

				if (cType == (uint16_t)ComponentType::ID)
				{
					// UUID
					uint64_t uuid = 0;
					if (!ReadAligned(entitySource, endED, uuid))
						return nullptr;

					scene->CreateEntityWithUUID(uuid, "Deserialized Entity");
					((SceneInfo&)assetInfo).m_Entities[uuid].InitializeData(eDataBuffer.GetSize(), eDataBuffer.As<uint8_t>());
				}
			}
			eDataBuffer.Release();
		}

		GE_CORE_INFO("AssetSerializer::DeserializeSceneFromPack(AssetInfo&) Successful");
		return scene;
	}

	bool AssetSerializer::DeserializeEntity(const SceneInfo::EntityInfo& eInfo, Entity& e)
	{
		if (eInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Entity from AssetPack.\n\EntityInfo has no Data");
			return false;
		}

		// Read Data & Assign
		const uint8_t* source = eInfo.DataBuffer.As<uint8_t>();
		const uint8_t* end = source + eInfo.DataBuffer.GetSize();

		while (source < end)
		{
			uint16_t currentType = 0;
			if (ReadAligned<uint16_t>(source, end, currentType))
			{
				switch (currentType)
				{
				case 0: // Invalid/None
				{
					GE_CORE_ERROR("Entity Component has invalid Type.");
					return false;
				}
				case 1: // ID
				{
					uint64_t uuid = 0;
					if (ReadAligned<uint64_t>(source, end, uuid))
						e.GetOrAddComponent<IDComponent>().ID = uuid;
				}
				break;
				case 2: // Tag
				{
					uint64_t tagStringSize = 0;
					if (!ReadAligned(source, end, tagStringSize))
						break;
					char* tagCStr = new char[tagStringSize];
					if (ReadAlignedArray<char>(source, end, tagCStr, tagStringSize))
						e.GetOrAddComponent<TagComponent>().Tag.assign(tagCStr, tagStringSize);
					delete[](tagCStr);
				}
				break;
				case 3: // Transform
				{
					TransformComponent tc = e.GetOrAddComponent<TransformComponent>();
					if (!ReadAlignedVec3<float>(source, end, tc.Translation.x, tc.Translation.y, tc.Translation.z))
					{
						GE_CORE_ERROR("Failed to read Entity TransformComponent Translation.");
					}

					if (!ReadAlignedVec3<float>(source, end, tc.Rotation.x, tc.Rotation.y, tc.Rotation.z))
					{
						GE_CORE_ERROR("Failed to read Entity TransformComponent Rotation.");
					}

					if (!ReadAlignedVec3<float>(source, end, tc.Scale.x, tc.Scale.y, tc.Scale.z))
					{
						GE_CORE_ERROR("Failed to read Entity TransformComponent Scale.");
					}
				}
				break;
				case 4: // Camera
				{
					CameraComponent cc = e.GetOrAddComponent<CameraComponent>();
					if (!ReadAligned<bool>(source, end, cc.Primary))
					{
						GE_CORE_ERROR("Failed to read CameraComponent Primary.");
					}

					if (!ReadAligned<bool>(source, end, cc.FixedAspectRatio))
					{
						GE_CORE_ERROR("Failed to read CameraComponent FixedAspectRatio.");
					}

					// SceneCamera Info
					float fov = 0;
					float nearClip = 0;
					float farClip = 0;
					if (ReadAligned<float>(source, end, fov) && ReadAligned<float>(source, end, nearClip) && ReadAligned<float>(source, end, farClip))
					{
						cc.ActiveCamera.SetInfo(fov, nearClip, farClip);
					}
					else
					{
						GE_CORE_ERROR("Failed to read CameraComponent SceneCamera Info.\n\tFOV:{0}\n\tNearClip:{1}\n\tFarClip:{2}", fov, nearClip, farClip);
					}
				}
				break;
				case 5: // AudioSource
				{
					AudioSourceComponent asc = e.GetOrAddComponent<AudioSourceComponent>();

					uint64_t uuid = 0;
					if (ReadAligned<uint64_t>(source, end, uuid))
						asc.AssetHandle = uuid;

					bool loop = false;
					if (ReadAligned<bool>(source, end, loop))
						asc.Loop = loop;

					float pitch = 0.0f;
					if (ReadAligned<float>(source, end, pitch))
						asc.Pitch = pitch;
					float gain = 0.0f;
					if (ReadAligned<float>(source, end, gain))
						asc.Gain = gain;
				}
				break;
				case 6: // AudioDevice
				{
					AudioListenerComponent alc = e.GetOrAddComponent<AudioListenerComponent>();
				}
					break;
				case 7: // SpriteRenderer
				{
					SpriteRendererComponent src = e.GetOrAddComponent<SpriteRendererComponent>();

					uint64_t uuid = 0;
					if (ReadAligned<uint64_t>(source, end, uuid))
						src.AssetHandle = uuid;

					float tilingFactor = 0;
					if (ReadAligned<float>(source, end, tilingFactor))
						src.TilingFactor = tilingFactor;

					float x, y, z, w = 0;
					if (ReadAlignedVec4<float>(source, end, x, y, z, w))
						src.Color = glm::vec4(x, y, z, w);
				}
				break;
				case 8: // CircleRenderer
				{
					CircleRendererComponent crc = e.GetOrAddComponent<CircleRendererComponent>();

					uint64_t uuid = 0;
					if (ReadAligned<uint64_t>(source, end, uuid))
						crc.AssetHandle = uuid;

					float tilingFactor = 0;
					if (ReadAligned<float>(source, end, tilingFactor))
						crc.TilingFactor = tilingFactor;

					float radius = 0;
					if (ReadAligned<float>(source, end, radius))
						crc.Radius = radius;

					float thickness = 0;
					if (ReadAligned<float>(source, end, thickness))
						crc.Thickness = thickness;

					float fade = 0;
					if (ReadAligned<float>(source, end, fade))
						crc.Fade = fade;

					float x, y, z, w = 0;
					if (ReadAlignedVec4<float>(source, end, x, y, z, w))
						crc.Color = glm::vec4(x, y, z, w);
				}
				break;
				case 9: // TextRenderer
				{
					TextRendererComponent trc = e.GetOrAddComponent<TextRendererComponent>();

					uint64_t uuid = 0;
					if (ReadAligned<uint64_t>(source, end, uuid))
						trc.AssetHandle = uuid;

					float kerningOffset = 0;
					if (ReadAligned<float>(source, end, kerningOffset))
						trc.KerningOffset = kerningOffset;

					float lineHeightOffset = 0;
					if (ReadAligned<float>(source, end, lineHeightOffset))
						trc.LineHeightOffset = lineHeightOffset;

					uint64_t textStringSize = 0;
					if (!ReadAligned(source, end, textStringSize))
						return false;
					char* textCStr = new char[textStringSize];
					if (ReadAlignedArray(source, end, textCStr, textStringSize))
						trc.Text.assign(textCStr, textStringSize);
					delete[](textCStr);

					float x, y, z, w = 0;
					if (ReadAlignedVec4<float>(source, end, x, y, z, w))
						trc.TextColor = glm::vec4(x, y, z, w);

					x, y, z, w = 0;
					if (ReadAlignedVec4<float>(source, end, x, y, z, w))
						trc.BGColor = glm::vec4(x, y, z, w);
				}
				break;
				case 10: // Rigidbody2D
				{
					Rigidbody2DComponent rb2dc = e.GetOrAddComponent<Rigidbody2DComponent>();

					uint16_t type = 0;
					if (ReadAligned<uint16_t>(source, end, type))
						rb2dc.Type = (Rigidbody2DComponent::BodyType)type; // TODO : Change to Physics::BodyType(?)

					bool fixedRotation = false;
					if (ReadAligned<bool>(source, end, fixedRotation))
						rb2dc.FixedRotation = fixedRotation;
				}
				break;
				case 11: // BoxCollider2D
				{
					BoxCollider2DComponent bc2dc = e.GetOrAddComponent<BoxCollider2DComponent>();

					bool show = false;
					if (ReadAligned<bool>(source, end, show))
						bc2dc.Show = show;

					float density = 0.0f;
					if (ReadAligned<float>(source, end, density))
						bc2dc.Density = density;

					float friction = 0.0f;
					if (ReadAligned<float>(source, end, friction))
						bc2dc.Friction = friction;

					float restitution = 0.0f;
					if (ReadAligned<float>(source, end, restitution))
						bc2dc.Restitution = restitution;

					float restitutionThreshold = 0.0f;
					if (ReadAligned<float>(source, end, restitutionThreshold))
						bc2dc.RestitutionThreshold = restitutionThreshold;

					float x, y = 0;
					if (ReadAlignedVec2<float>(source, end, x, y))
						bc2dc.Offset = glm::vec2(x, y);

					x, y = 0;
					if (ReadAlignedVec2<float>(source, end, x, y))
						bc2dc.Size = glm::vec2(x, y);
				}
				break;
				case 12: // CircleCollider2D
				{
					CircleCollider2DComponent cc2dc = e.GetOrAddComponent<CircleCollider2DComponent>();

					bool show = false;
					if (ReadAligned<bool>(source, end, show))
						cc2dc.Show = show;

					float radius = 0.0f;
					if (ReadAligned<float>(source, end, radius))
						cc2dc.Radius = radius;

					float density = 0.0f;
					if (ReadAligned<float>(source, end, density))
						cc2dc.Density = density;

					float friction = 0.0f;
					if (ReadAligned<float>(source, end, friction))
						cc2dc.Friction = friction;

					float restitution = 0.0f;
					if (ReadAligned<float>(source, end, restitution))
						cc2dc.Restitution = restitution;

					float restitutionThreshold = 0.0f;
					if (ReadAligned<float>(source, end, restitutionThreshold))
						cc2dc.RestitutionThreshold = restitutionThreshold;

					float x, y = 0;
					if (ReadAlignedVec2<float>(source, end, x, y))
						cc2dc.Offset = glm::vec2(x, y);
				}
				break;
				case 13: // NativeScript
				{
					NativeScriptComponent nsc = e.GetOrAddComponent<NativeScriptComponent>();

				}
				break;
				case 14: // Script
				{
					ScriptComponent sc = e.GetOrAddComponent<ScriptComponent>();

					uint64_t classNameStringSize = 0;
					if (!ReadAligned(source, end, classNameStringSize))
						break;
					char* classNameCStr = new char[classNameStringSize];
					if (ReadAlignedArray<char>(source, end, classNameCStr, classNameStringSize))
						sc.ClassName.assign(classNameCStr, classNameStringSize);
					delete[](classNameCStr);
				}
				break;
				default:
					GE_CORE_ERROR("Entity Component has invalid Type.");
					return false;
					break;
				}

				// Overflow check
				if (source > end)
				{
					GE_CORE_ASSERT(false, "AssetSerializer::DeserializeEntity(const EntityInfo&, Entity&) Overflow");
					return false;
				}
			}
			else
			{
				GE_CORE_ERROR("Failed to read Entity's ComponentType.");
				return false;
			}
		}

		GE_CORE_INFO("AssetSerializer::DeserializeEntity(const EntityInfo&, Entity&) Successful");
		return true;
	}

	Ref<Asset> AssetSerializer::DeserializeTexture2DFromPack(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Texture2D from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		Texture::Config config;

		// Read Data & Assign
		const uint8_t* source = assetInfo.DataBuffer.As<uint8_t>();
		const uint8_t* end = source + assetInfo.DataBuffer.GetSize();

		// Handle
		uint64_t handle = 0;
		if (!ReadAligned<uint64_t>(source, end, handle))
			return nullptr;

		// Type has already been read, skip 
		source += GetAligned(sizeof(uint16_t));

		// Name
		uint64_t sizeofString = 0;
		if (!ReadAligned(source, end, sizeofString))
			return nullptr;
		char* name = new char[sizeofString];
		if (!ReadAlignedArray<char>(source, end, name, sizeofString))
			return nullptr;
		config.Name.assign(name, sizeofString);
		delete[](name);

		// Config
		if (!ReadAligned<uint32_t>(source, end, config.Width))
			return nullptr;
		if (!ReadAligned<uint32_t>(source, end, config.Height))
			return nullptr;

		uint16_t internalFormat = 0;
		if (!ReadAligned<uint16_t>(source, end, internalFormat))
			return nullptr;
		config.InternalFormat = (Texture::ImageFormat)internalFormat;

		uint16_t format = 0;
		if(!ReadAligned<uint16_t>(source, end, format))
			return nullptr;
		config.Format = (Texture::DataFormat)format;

		if (!ReadAligned<bool>(source, end, config.GenerateMips))
			return nullptr;

		// Buffer Data
		uint64_t textureDataSize = 0;
		if (!ReadAligned(source, end, textureDataSize))
			return nullptr;
		uint8_t* data = new uint8_t[textureDataSize];
		if (!ReadAlignedArray<uint8_t>(source, end, data, textureDataSize))
			return nullptr;

		Buffer textureBuffer = Buffer(data, textureDataSize);
		delete[] data;

		// Overflow check
		if (source > end)
			GE_CORE_ASSERT(false, "AssetSerializer::DeserializeTexture2DFromPack(AssetInfo&) Buffer Overflow");

		// Set
		Ref<Texture2D> texture = Texture2D::Create(config, textureBuffer);
		texture->p_Handle = handle;
		textureBuffer.Release();
		GE_CORE_INFO("AssetSerializer::DeserializeTexture2DFromPack(AssetInfo&) Successful");
		return texture;
	}
	
	Ref<Asset> AssetSerializer::DeserializeFontFromPack(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Font from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		/*
		* - Handle
		* - Type
		* - Name
		* ~ Config
		* * - Width : uint32_t
		* * - Height : uint32_t
		* * - Scale : float
		* * - Seed : uint64_t
		* * - ThreadCount : uint32_t
		* * - ExpensiveColoring : bool
		* * ~ FontAtlasBuffer : Texture2D Buffer
		* * * - FontAtlasSize
		* * * - FontAtlasBuffer
		*/

		Font::AtlasConfig config;

		// Read Data & Assign
		const uint8_t* source = assetInfo.DataBuffer.As<uint8_t>();
		const uint8_t* end = source + assetInfo.DataBuffer.GetSize();

		// Handle
		uint64_t handle = 0;
		if (!ReadAligned<uint64_t>(source, end, handle))
			return nullptr;

		// Type has already been read, skip 
		source += GetAligned(sizeof(uint16_t));

		// Name
		uint64_t sizeofString = 0;
		if (!ReadAligned(source, end, sizeofString))
			return nullptr;
		char* name = new char[sizeofString];
		if (!ReadAlignedArray<char>(source, end, name, sizeofString))
			return nullptr;
		config.Name.assign(name, sizeofString);
		delete[](name);

		// Config
		uint32_t width = 0;
		if (!ReadAligned<uint32_t>(source, end, width))
			return nullptr;
		uint32_t height = 0;
		if (!ReadAligned<uint32_t>(source, end, height))
			return nullptr;
		if (!ReadAligned<float>(source, end, config.Scale))
			return nullptr;
		if (!ReadAligned<uint64_t>(source, end, config.Seed))
			return nullptr;
		if (!ReadAligned<uint32_t>(source, end, config.ThreadCount))
			return nullptr;
		if (!ReadAligned<bool>(source, end, config.ExpensiveColoring))
			return nullptr;

		// FontAtlasTexture Size & Data
		size_t size = 0;
		if (!ReadAligned(source, end, size))
			return nullptr;
		uint8_t* data = new uint8_t[size];
		if (!ReadAlignedArray<uint8_t>(source, end, data, size))
			return nullptr;
		
		Buffer textBuffer = Buffer(data, size);
		delete[] data;

		// Overflow check
		if (source > end)
			GE_CORE_ASSERT(false, "AssetSerializer::DeserializeFont2DFromPack(AssetInfo&) Buffer Overflow");
		
		// Create & Assign Data
		Ref<Font> font = CreateRef<Font>(handle, config);
		font->p_Status = Asset::Status::Loading;
		font->m_AtlasConfig.Texture = LoadFontAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>
			(textBuffer, width, height, font->m_AtlasConfig, font->m_MSDFData);
		textBuffer.Release();
		font->p_Status = Asset::Status::Ready;

		return font;
	}

	Ref<Asset> AssetSerializer::DeserializeAudioFromPack(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Font from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		/*
		* - Handle
		* - Type
		* - Name
		* ~ Config
		* * - Loop : bool
		* * - Pitch : float
		* * - Gain : float
		* * - Channels : uint8_t
		* * - SampleRate : uint32_t
		* * - BPS : uint8_t
		* * - Format : uint32_t
		* * ~ Audio Buffer Data
		* * * - Size : uint32_t
		* * * ~ Data : uint_8_t*
		*/

		// Read Data & Assign
		const uint8_t* source = assetInfo.DataBuffer.As<uint8_t>();
		const uint8_t* end = source + assetInfo.DataBuffer.GetSize();

		// Handle
		uint64_t handle = 0;
		if (!ReadAligned<uint64_t>(source, end, handle))
			return nullptr;

		// Type has already been read, skip 
		source += GetAligned(sizeof(uint16_t));

		// Name
		uint64_t sizeofString = 0;
		if (!ReadAligned(source, end, sizeofString))
			return nullptr;
		char* name = new char[sizeofString];
		if (!ReadAlignedArray<char>(source, end, name, sizeofString))
			return nullptr;

		// Config
		bool loop = false;
		if (!ReadAligned<bool>(source, end, loop))
			return nullptr;
		float pitch = 1.0f;
		if(!ReadAligned<float>(source, end, pitch))
			return nullptr;
		float gain = 1.0f;
		if(!ReadAligned<float>(source, end, gain))
			return nullptr;

		uint8_t channels = 0;
		if(!ReadAligned<uint8_t>(source, end, channels))
			return nullptr;
		uint32_t sampleRate = 0;
		if(!ReadAligned<uint32_t>(source, end, sampleRate))
			return nullptr;
		uint8_t bps = 0;
		if(!ReadAligned<uint8_t>(source, end, bps))
			return nullptr;
		uint32_t format = 0;
		if(!ReadAligned<uint32_t>(source, end, format))
			return nullptr;

		// Audio Buffer
		size_t size = 0;
		if (!ReadAligned(source, end, size))
			return nullptr;
		uint8_t* data = new uint8_t[size];
		if (!ReadAlignedArray<uint8_t>(source, end, data, size))
			return nullptr;
		
		Buffer buffer = Buffer(data, size);
		delete[] data;

		// Overflow check
		if (source > end)
			GE_CORE_ASSERT(false, "AssetSerializer::DeserializeAudioFromPack(AssetInfo&) Buffer Overflow");

		std::string stringName = std::string(name, sizeofString);
		delete[](name);
		Ref<Audio> audio = Audio::Create(handle, Audio::Config(stringName, channels, sampleRate, bps, buffer));
		buffer.Release();
		GE_CORE_INFO("AssetSerializer::DeserializeAudioFromPack(AssetInfo&) Successful");
		return audio;
	}
	
	bool AssetSerializer::SerializeSceneForPack(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		// ScenePackFormat
		//      [84 + ?] SceneInfo  : Value, corresponding Key handled in SerializePack
		//          [8] Packed Size : Size of whole Scene
		//			[76 + ?] Data
		//				[8] Handle
		//				[2] Type
		//				[8] Name
		//				[8] Asset Map Count
		//				[26 + ?] Asset Map			// Size based on how many Assets are loaded
		//				    Asset Handle    : Key
		//				    [26 + ?] AssetInfo	: Value
		//				        [8] Packed Size
		//						[18 + ?] Packed Data
		//							[8] Handle
		//							[2] Type
		//							[8] Name
		//							[?] Asset Specific Info : See DeserializeAsset(AssetInfo&)
		//				[8] Entity Map Count
		//				[16 + ?] Entity Map			// Size based on how many Entities are loaded
		//				    Handle			: Key
		//				    [8 + ?] EntityInfo	: Value
		//				        [8] Packed Size
		//						[?]	Packed Data
		//							[?] Entity Specific Info : See DeserializeEntity(const EntityInfo&, Entity&)

		Ref<Scene> scene = Project::GetAssetAs<Scene>(asset);
		if (!scene)
			return false;
		SceneInfo sceneInfo = *(SceneInfo*)&assetInfo;
		uint64_t requiredSize = 0;

		// Calculate Size
		{
			requiredSize += GetAligned(sizeof(scene->p_Handle)) // sizeof(uint64_t)
				+ GetAligned(sizeof(scene->p_Type))	// sizeof(uint16_t)
				+ GetAlignedOfArray<char>(scene->m_Config.Name.size())
				+ GetAligned(sizeof(scene->m_Config.StepFrames)); // sizeof(uint64_t)

			// For Assets
			{
				AssetMap assetMap = Project::GetAssetManager<RuntimeAssetManager>()->GetLoadedAssets();
				requiredSize += GetAligned(sizeof(assetMap.size())); //sizeof(assetCount)
				for (const auto& [uuid, asset] : assetMap)
				{
					if (uuid == scene->p_Handle || asset->GetType() == Asset::Type::Scene)
						continue;

					if (SerializeAsset(asset, sceneInfo.m_Assets[uuid]))
					{
						// += SizeofData + Data
						requiredSize += GetAlignedOfArray<uint8_t>(sceneInfo.m_Assets.at(uuid).DataBuffer.GetSize());
					}
				}
			}

			// For Entities
			{
				requiredSize += GetAligned(sizeof(uint64_t)); //sizeof(eCount)
				{
					// Every Entity needs an ID
					auto idView = scene->GetRegistry().view<IDComponent>();
					for (auto e : idView)
					{
						Entity e(e, scene.get());
						UUID uuid = e.GetComponent<IDComponent>().ID;
						if (SerializeEntity(sceneInfo.m_Entities[uuid], e))
						{
							// += SizeofData + Data
							requiredSize += GetAlignedOfArray<uint8_t>(sceneInfo.m_Entities.at(uuid).DataBuffer.GetSize());
						}

					}

				}

			}
		}

		// Allocate Size for Data
		assetInfo.InitializeData(requiredSize);

		// Set Data
		{
			if (assetInfo.DataBuffer)
			{
				if (assetInfo.DataBuffer.GetSize() >= requiredSize)
				{
					// Start at beginning of buffer
					uint8_t* destination = assetInfo.DataBuffer.As<uint8_t>();
					uint8_t* end = destination + requiredSize;

					// Clear requiredSize from destination
					memset(destination, 0, requiredSize);

					// Fill out buffer
					{
						WriteAligned<uint64_t>(destination, scene->p_Handle);
						WriteAligned<uint16_t>(destination, (uint16_t)scene->p_Type);

						const char* cStr = scene->m_Config.Name.c_str();
						WriteAlignedArray<char>(destination, cStr, (uint64_t)scene->m_Config.Name.size());

						WriteAligned<uint64_t>(destination, scene->m_Config.StepFrames);

						// For Assets
						{
							WriteAligned<uint64_t>(destination, sceneInfo.m_Assets.size());

							for (const auto& [uuid, assetInfo] : sceneInfo.m_Assets)
							{
								//  asset size & data
								const uint8_t* data = assetInfo.DataBuffer.As<uint8_t>();
								WriteAlignedArray<uint8_t>(destination, data, assetInfo.DataBuffer.GetSize());

							}

						}

						// For Entities
						{
							WriteAligned<uint64_t>(destination, sceneInfo.m_Entities.size());

							for (const auto& [uuid, eInfo] : sceneInfo.m_Entities)
							{
								// e size & data
								const uint8_t* data = eInfo.DataBuffer.As<uint8_t>();
								WriteAlignedArray<uint8_t>(destination, data, eInfo.DataBuffer.GetSize());
							}
						}
					}

					if (destination - assetInfo.DataBuffer.As<uint8_t>() == requiredSize)
					{
						GE_CORE_INFO("AssetSerializer::SerializeSceneForPack Successful.");
					}
					else
					{
						GE_CORE_ASSERT(false, "Buffer overflow.");
						return false;
					}
				}
				else
				{
					GE_CORE_ERROR("Required size is larger than given buffer size.");
					return false;
				}
			}

		}
		
		// assetInfo has all Serialized Data.
		sceneInfo.ClearAllData();
		return true;

	}

	bool AssetSerializer::SerializeEntity(SceneInfo::EntityInfo& eInfo, const Entity& e)
	{
		uint64_t requiredSize = 0;

		// Get Size
		{
			// ID Component
			if (!e.HasComponent<IDComponent>())
			{
				GE_CORE_ERROR("Cannot serialize Entity for pack. Entity does not have ID.");
				return false;
			}
			// ComponentType as uint16_t + IDComponent UUID  
			requiredSize += GetAligned(sizeof(uint16_t)) + GetAligned(sizeof(uint64_t));

			// Tag Component 
			if (e.HasComponent<TagComponent>())
			{
				TagComponent tc = e.GetComponent<TagComponent>();
				// Component identifier + Tag String
				requiredSize += GetAligned(sizeof(uint16_t)) + GetAlignedOfArray<char>(tc.Tag.size());
			}

			// Transform
			if (e.HasComponent<TransformComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				// Translation
				requiredSize += GetAlignedOfVec3<float>();

				// Rotation
				requiredSize += GetAlignedOfVec3<float>();

				// Scale
				requiredSize += GetAlignedOfVec3<float>();
			}

			// Camera
			if (e.HasComponent<CameraComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				requiredSize += GetAligned(sizeof(bool))	// Primary
					+ GetAligned(sizeof(bool));			// FixedAspectRatio

				// SceneCamera Variables
				requiredSize += GetAligned(sizeof(float))	// FOV
					+ GetAligned(sizeof(float))			// NearCip
					+ GetAligned(sizeof(float));			// FarCip
			}

			// AudioSource
			if (e.HasComponent<AudioSourceComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				requiredSize += GetAligned(sizeof(uint64_t))	// Audio Asset UUID, See Assets(UUID)
					+ GetAligned(sizeof(bool))					// Loop
					+ GetAligned(sizeof(float))				// Pitch
					+ GetAligned(sizeof(float));				//Gain

			}

			// AudioDevice
			if (e.HasComponent<AudioListenerComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));
			}

			// SpriteRenderer
			if (e.HasComponent<SpriteRendererComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				SpriteRendererComponent src = e.GetComponent<SpriteRendererComponent>();

				requiredSize += GetAligned(sizeof(uint64_t))	// Texture Asset UUID
					+ GetAligned(sizeof(float));				// TilingFactor

				// Color
				requiredSize += GetAlignedOfVec4<float>();
			}

			// CircleRenderer
			if (e.HasComponent<CircleRendererComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				CircleRendererComponent crc = e.GetComponent<CircleRendererComponent>();

				requiredSize += GetAligned(sizeof(uint64_t))	// Texture Asset UUID
					+ GetAligned(sizeof(float))				// TilingFactor
					+ GetAligned(sizeof(float))				// Radius
					+ GetAligned(sizeof(float))				// Thickness
					+ GetAligned(sizeof(float));				// Fade

				// Color
				requiredSize += GetAlignedOfVec4<float>();
			}

			// TextRenderer
			if (e.HasComponent<TextRendererComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				TextRendererComponent trc = e.GetComponent<TextRendererComponent>();

				requiredSize += GetAligned(sizeof(uint64_t))		// Texture Asset UUID
					+ GetAligned(sizeof(trc.KerningOffset))			// KerningOffset
					+ GetAligned(sizeof(trc.LineHeightOffset))		// LineHeightOffset
					+ GetAlignedOfArray<char>(trc.Text.size());		// size of Text String & Text String

				// TextColor
				requiredSize += GetAlignedOfVec4<float>();

				// BGColor
				requiredSize += GetAlignedOfVec4<float>();
			}

			// Rigidbody2D
			if (e.HasComponent<Rigidbody2DComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				requiredSize += GetAligned(sizeof(uint16_t))	// Type
					+ GetAligned(sizeof(bool));					// FixedRotation
			}

			// BoxCollider2D
			if (e.HasComponent<BoxCollider2DComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				requiredSize += GetAligned(sizeof(bool))	// Show
					+ GetAligned(sizeof(float))			// Density
					+ GetAligned(sizeof(float))			// Friction
					+ GetAligned(sizeof(float))			// Restitution
					+ GetAligned(sizeof(float));			// RestitutionThreshold

				// Offset
				requiredSize += GetAlignedOfVec2<float>();

				// Size
				requiredSize += GetAlignedOfVec2<float>();

			}

			// CircleCollider2D
			if (e.HasComponent<CircleCollider2DComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				requiredSize += GetAligned(sizeof(bool))	// Show
					+ GetAligned(sizeof(float))				// Radius
					+ GetAligned(sizeof(float))				// Density
					+ GetAligned(sizeof(float))				// Friction
					+ GetAligned(sizeof(float))				// Restitution
					+ GetAligned(sizeof(float));			// RestitutionThreshold

				// Offset
				requiredSize += GetAlignedOfVec2<float>();
			}

			// NativeScript
			if (e.HasComponent<NativeScriptComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));
			}

			// Script
			if (e.HasComponent<ScriptComponent>())
			{
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				ScriptComponent sc = e.GetComponent<ScriptComponent>();

				// TODO : Change to UUID from Script : Asset
				requiredSize += GetAlignedOfArray<char>(sc.ClassName.size());
			}
		}

		// Allocate Size for Data
		eInfo.DataBuffer.Allocate(requiredSize);

		// Set Data
		if (eInfo.DataBuffer.As<uint8_t>())
		{
			if (eInfo.DataBuffer.GetSize() >= requiredSize)
			{
				// Start at beginning of buffer
				uint8_t* destination = eInfo.DataBuffer.As<uint8_t>();
				uint8_t* end = destination + requiredSize;

				// Clear requiredSize from destination
				memset(destination, 0, requiredSize);

				// Fill out buffer in order. Based on Size step.
				{
					// ID Component
					{
						uint16_t currentType = (uint16_t)ComponentType::ID;
						WriteAligned(destination, currentType);

						uint64_t uuid = e.GetComponent<IDComponent>().ID;
						WriteAligned(destination, uuid);
					}

					// All Other Components
					{
						// Tag
						if (e.HasComponent<TagComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::Tag;
							WriteAligned(destination, currentType);

							TagComponent tc = e.GetComponent<TagComponent>();

							const char* tagCStr = tc.Tag.c_str();
							WriteAlignedArray<char>(destination, tagCStr, tc.Tag.size());

						}

						// Transform
						if (e.HasComponent<TransformComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::Transform;
							WriteAligned(destination, currentType);

							TransformComponent trsc = e.GetComponent<TransformComponent>();

							// Translation
							WriteAlignedVec3<float>(destination, trsc.Translation.x, trsc.Translation.y, trsc.Translation.z);

							// Rotation
							WriteAlignedVec3<float>(destination, trsc.Rotation.x, trsc.Rotation.y, trsc.Rotation.z);

							// Scale
							WriteAlignedVec3<float>(destination, trsc.Scale.x, trsc.Scale.y, trsc.Scale.z);

						}

						// Camera
						if (e.HasComponent<CameraComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::Camera;
							WriteAligned(destination, currentType);

							CameraComponent cc = e.GetComponent<CameraComponent>();

							WriteAligned<bool>(destination, cc.Primary);
							WriteAligned<bool>(destination, cc.FixedAspectRatio);

							// SceneCamera Variables
							WriteAligned<float>(destination, cc.ActiveCamera.GetFOV());
							WriteAligned<float>(destination, cc.ActiveCamera.GetNearClip());
							WriteAligned<float>(destination, cc.ActiveCamera.GetFarClip());

						}

						// AudioSource
						if (e.HasComponent<AudioSourceComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::AudioSource;
							WriteAligned(destination, currentType);

							AudioSourceComponent asc = e.GetComponent<AudioSourceComponent>();

							WriteAligned<uint64_t>(destination, asc.AssetHandle);

							WriteAligned<bool>(destination, asc.Loop);

							WriteAligned<float>(destination, asc.Pitch);
							WriteAligned<float>(destination, asc.Gain);

						}

						// AudioDevice
						if (e.HasComponent<AudioListenerComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::AudioListener;
							WriteAligned(destination, currentType);

							AudioListenerComponent alc = e.GetComponent<AudioListenerComponent>();

						}

						// SpriteRenderer
						if (e.HasComponent<SpriteRendererComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::SpriteRenderer;
							WriteAligned(destination, currentType);

							SpriteRendererComponent src = e.GetComponent<SpriteRendererComponent>();

							WriteAligned<uint64_t>(destination, src.AssetHandle);

							WriteAligned<float>(destination, src.TilingFactor);

							// Color
							WriteAlignedVec4<float>(destination, src.Color.a, src.Color.g, src.Color.b, src.Color.a);

						}

						// CircleRenderer
						if (e.HasComponent<CircleRendererComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::CircleRenderer;
							WriteAligned(destination, currentType);

							CircleRendererComponent crc = e.GetComponent<CircleRendererComponent>();

							WriteAligned<uint64_t>(destination, crc.AssetHandle);

							WriteAligned<float>(destination, crc.TilingFactor);
							WriteAligned<float>(destination, crc.Radius);
							WriteAligned<float>(destination, crc.Thickness);
							WriteAligned<float>(destination, crc.Fade);

							// Color
							WriteAlignedVec4<float>(destination, crc.Color.a, crc.Color.g, crc.Color.b, crc.Color.a);
						}

						// TextRenderer
						if (e.HasComponent<TextRendererComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::TextRenderer;
							WriteAligned(destination, currentType);

							TextRendererComponent trc = e.GetComponent<TextRendererComponent>();

							WriteAligned<uint64_t>(destination, trc.AssetHandle);

							WriteAligned<float>(destination, trc.KerningOffset);
							WriteAligned<float>(destination, trc.LineHeightOffset);

							// Text
							const char* trcTextCStr = trc.Text.c_str();
							WriteAlignedArray<char>(destination, trcTextCStr, trc.Text.size());

							// Color
							WriteAlignedVec4<float>(destination, trc.TextColor.a, trc.TextColor.g, trc.TextColor.b, trc.TextColor.a);

							WriteAlignedVec4<float>(destination, trc.BGColor.a, trc.BGColor.g, trc.BGColor.b, trc.BGColor.a);
						}

						// Rigidbody2D
						if (e.HasComponent<Rigidbody2DComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::Rigidbody2D;
							WriteAligned(destination, currentType);

							Rigidbody2DComponent rb2D = e.GetComponent<Rigidbody2DComponent>();

							uint16_t typeInt = (uint16_t)rb2D.Type;
							WriteAligned<uint16_t>(destination, typeInt);

							WriteAligned<bool>(destination, rb2D.FixedRotation);
						}

						// BoxCollider2D
						if (e.HasComponent<BoxCollider2DComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::BoxCollider2D;
							WriteAligned(destination, currentType);

							BoxCollider2DComponent bc2d = e.GetComponent<BoxCollider2DComponent>();

							WriteAligned<bool>(destination, bc2d.Show);

							WriteAligned<float>(destination, bc2d.Density);
							WriteAligned<float>(destination, bc2d.Friction);
							WriteAligned<float>(destination, bc2d.Restitution);
							WriteAligned<float>(destination, bc2d.RestitutionThreshold);

							// Offset
							WriteAlignedVec2<float>(destination, bc2d.Offset.x, bc2d.Offset.y);

							// Size
							WriteAlignedVec2<float>(destination, bc2d.Size.x, bc2d.Size.y);

						}

						// CircleCollider2D
						if (e.HasComponent<CircleCollider2DComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::CircleCollider2D;
							WriteAligned(destination, currentType);

							CircleCollider2DComponent cc2d = e.GetComponent<CircleCollider2DComponent>();

							WriteAligned<bool>(destination, cc2d.Show);

							WriteAligned<float>(destination, cc2d.Radius);
							WriteAligned<float>(destination, cc2d.Density);
							WriteAligned<float>(destination, cc2d.Friction);
							WriteAligned<float>(destination, cc2d.Restitution);
							WriteAligned<float>(destination, cc2d.RestitutionThreshold);

							// Offset
							WriteAlignedVec2<float>(destination, cc2d.Offset.x, cc2d.Offset.y);

						}

						// NativeScript
						if (e.HasComponent<NativeScriptComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::NativeScript;
							WriteAligned(destination, currentType);
						}

						// Script
						if (e.HasComponent<ScriptComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::Script;
							WriteAligned(destination, currentType);

							ScriptComponent sc = e.GetComponent<ScriptComponent>();

							const char* classNameCStr = sc.ClassName.c_str();
							WriteAlignedArray<char>(destination, classNameCStr, sc.ClassName.size());

							// TODO : Add other script variables (?) || switch to UUID & create Script : Asset
						}


					}
				}

				if (destination - eInfo.DataBuffer.As<uint8_t>() == requiredSize)
				{
					GE_CORE_INFO("AssetSerializer::SerializeEntity(EntityInfo&, const Entity&) Successful.");
					return true;
				}
				else
				{
					GE_CORE_ASSERT(false, "AssetSerializer::SerializeEntity(EntityInfo&, const Entity&) Buffer Overflow.");
				}
			}
			else
			{
				GE_CORE_ERROR("AssetSerializer::SerializeEntity(EntityInfo&, const Entity&)\n\tRequired size is larger than given buffer size.");
			}
		}

		return false;
	}

	bool AssetSerializer::SerializeTexture2DForPack(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		Ref<Texture2D> texture = Project::GetAssetAs<Texture2D>(asset);
		assetInfo.Type = 2; // See Asset::Type

		uint64_t requiredSize = 0;

		// Size
		{
			requiredSize = GetAligned(sizeof(uint64_t)) // Handle
				+ GetAligned(sizeof(uint16_t))	// Type
				+ GetAlignedOfArray<char>(texture->GetConfig().Name.size()); // sizeofString + String

			// Config
			requiredSize += GetAligned(sizeof(uint32_t)) // Width
				+ GetAligned(sizeof(uint32_t))  // Height
				+ GetAligned(sizeof(uint16_t)) // InternalFormat
				+ GetAligned(sizeof(uint16_t))	// Format
				+ GetAligned(sizeof(bool)); // GenerateMips

			// Buffer Size & Data
			requiredSize += GetAlignedOfArray<uint8_t>(texture->GetConfig().TextureBuffer.GetSize());
		}

		// Allocate Size for Data
		assetInfo.InitializeData(requiredSize);

		// Data
		if (assetInfo.DataBuffer)
		{
			if (assetInfo.DataBuffer.GetSize() >= requiredSize)
			{
				// Start at beginning of buffer
				uint8_t* destination = assetInfo.DataBuffer.As<uint8_t>();
				uint8_t* end = destination + requiredSize;

				// Clear requiredSize from destination
				memset(destination, 0, requiredSize);

				// Fill out buffer
				{
					WriteAligned<uint64_t>(destination, texture->p_Handle);
					WriteAligned<uint16_t>(destination, (uint16_t)texture->p_Type);

					const char* cStr = texture->GetConfig().Name.c_str();
					WriteAlignedArray<char>(destination, cStr, texture->GetConfig().Name.size());

					// Texture::Config
					{
						WriteAligned<uint32_t>(destination, texture->GetConfig().Width);

						WriteAligned<uint32_t>(destination, texture->GetConfig().Height);

						WriteAligned<uint16_t>(destination, (uint16_t)texture->GetConfig().InternalFormat);

						WriteAligned<uint16_t>(destination, (uint16_t)texture->GetConfig().Format);

						WriteAligned<bool>(destination, texture->GetConfig().GenerateMips);

					}

					// Texture Buffer Size & Data
					const uint8_t* data = texture->GetConfig().TextureBuffer.As<uint8_t>();
					WriteAlignedArray<uint8_t>(destination, data, (uint64_t)texture->GetConfig().TextureBuffer.GetSize());

				}

				if (destination - assetInfo.DataBuffer.As<uint8_t>() == requiredSize)
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
		* * - Width : uint32_t
		* * - Height : uint32_t
		* * - Scale : float
		* * - Seed : uint64_t
		* * - ThreadCount : uint32_t
		* * - ExpensiveColoring : bool
		* * ~ FontAtlasBuffer : Texture2D Buffer
		* * * * - Size
		* * * * - Data
		*/

		Ref<Font> font = Project::GetAssetAs<Font>(asset);
		assetInfo.Type = 3; // See Asset::Type

		Ref<Texture2D> atlasTexture = font->GetAtlasTexture();
		if (!atlasTexture)
		{
			GE_CORE_ERROR("Cannot Serialize Font Asset that does not have Atlas Texture Data");
			return false;
		}

		uint64_t requiredSize = 0;

		// Size
		{
			requiredSize = GetAligned(sizeof(uint64_t))							// Handle
				+ GetAligned(sizeof(uint16_t))									// Type
				+ GetAlignedOfArray<char>(font->m_AtlasConfig.Name.size());		// size of String & String

			// Font Config
			requiredSize += GetAligned(sizeof(uint32_t))	// Width
				+ GetAligned(sizeof(uint32_t))				// Height
				+ GetAligned(sizeof(float))				// Scale
				+ GetAligned(sizeof(uint64_t))				// Seed
				+ GetAligned(sizeof(uint32_t))				// Thread Count
				+ GetAligned(sizeof(bool));				// ExpensiveColoring

			// Special case for Font Atlas Texture Size & Data. Not handled with SerializeAsset(Ref<Asset>, AssetInfo&)
			requiredSize += GetAlignedOfArray<uint8_t>(atlasTexture->GetConfig().TextureBuffer.GetSize());
		}

		// Allocate Size for Data
		assetInfo.InitializeData(requiredSize);

		// Data
		if (assetInfo.DataBuffer)
		{
			if (assetInfo.DataBuffer.GetSize() >= requiredSize)
			{
				// Start at beginning of buffer
				uint8_t* destination = assetInfo.DataBuffer.As<uint8_t>();
				uint8_t* end = destination + requiredSize;

				// Clear requiredSize from destination
				memset(destination, 0, requiredSize);

				// Fill out buffer

				WriteAligned<uint64_t>(destination, font->p_Handle);
				WriteAligned<uint16_t>(destination, (uint16_t)font->p_Type);

				const char* cStr = font->m_AtlasConfig.Name.c_str();
				WriteAlignedArray<char>(destination, cStr, (uint64_t)font->m_AtlasConfig.Name.size());

				// Config
				{
					const Texture::Config& textureConfig = atlasTexture->GetConfig();
					WriteAligned<uint32_t>(destination, textureConfig.Width);
					WriteAligned<uint32_t>(destination, textureConfig.Height);
					WriteAligned<float>(destination, font->m_AtlasConfig.Scale);
					WriteAligned<uint64_t>(destination, font->m_AtlasConfig.Seed);
					WriteAligned<uint32_t>(destination, font->m_AtlasConfig.ThreadCount);
					WriteAligned<bool>(destination, font->m_AtlasConfig.ExpensiveColoring);

					// FontTexture Size & Data
					const uint8_t* data = textureConfig.TextureBuffer.As<uint8_t>();
					WriteAlignedArray<uint8_t>(destination, data, atlasTexture->GetConfig().TextureBuffer.GetSize());
				}

				if (destination - assetInfo.DataBuffer.As<uint8_t>() == requiredSize)
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
		* * - Loop : bool
		* * - Pitch : float
		* * - Gain : float
		* * - Channels : uint8_t
		* * - SampleRate : uint32_t
		* * - BPS : uint8_t
		* * - Format : uint32_t
		* * ~ Audio Buffer Data
		* * * - Size : uint32_t
		* * * ~ Data : uint_8_t*
		*/

		Ref<Audio> audio = Project::GetAssetAs<Audio>(asset);
		assetInfo.Type = 4; // See Asset::Type::Audio
		const Audio::Config& config = audio->GetConfig();

		uint64_t requiredSize = 0;

		// Size
		{
			requiredSize = GetAligned(sizeof(uint64_t)) // Handle
				+ GetAligned(sizeof(uint16_t))	// Type
				+ GetAligned(sizeof(size_t)) // sizeof(stringLength)
				+ GetAligned(config.Name.size() * sizeof(char)); //sizeof(string)

			// Config
			requiredSize += GetAligned(sizeof(bool))       // Loop
				+ GetAligned(sizeof(float))                // Pitch
				+ GetAligned(sizeof(float));               // Gain

			requiredSize += GetAligned(sizeof(uint8_t))		// Channels
				+ GetAligned(sizeof(uint32_t))               // SampleRate
				+ GetAligned(sizeof(uint8_t))               // BPS
				+ GetAligned(sizeof(uint32_t));              // Format

			if (config.DataBuffer)
			{
			    requiredSize += GetAligned(sizeof(uint64_t))							// Size of data
			        + GetAligned(config.DataBuffer.GetSize() * sizeof(uint8_t));	// Data
			}

		}

		// Allocate Size for Data
		assetInfo.InitializeData(requiredSize);

		// Data
		if (assetInfo.DataBuffer)
		{
			if (assetInfo.DataBuffer.GetSize() >= requiredSize)
			{

				// Start at beginning of buffer
				uint8_t* destination = assetInfo.DataBuffer.As<uint8_t>();
				uint8_t* end = destination + requiredSize;

				// Clear requiredSize from destination
				memset(destination, 0, requiredSize);

				// Fill out buffer
				{
					WriteAligned<uint64_t>(destination, audio->p_Handle);
					WriteAligned<uint16_t>(destination, (uint16_t)audio->p_Type);

					const char* cStr = config.Name.c_str();
					WriteAlignedArray<char>(destination, cStr, (uint64_t)config.Name.size());

					// Config
					{
						WriteAligned<uint8_t>(destination, config.Channels);
						WriteAligned<uint32_t>(destination, config.SampleRate);
						WriteAligned<uint8_t>(destination, config.BPS);
						WriteAligned<uint32_t>(destination, config.Format);

						if (config.DataBuffer)
						{
							const uint8_t* data = config.DataBuffer.As<uint8_t>();
							WriteAlignedArray<uint8_t>(destination, data, config.DataBuffer.GetSize());
						}
					}

				}

				if (destination - assetInfo.DataBuffer.As<uint8_t>() == requiredSize)
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

#pragma endregion
}

		