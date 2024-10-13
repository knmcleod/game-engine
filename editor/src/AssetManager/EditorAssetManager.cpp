#include "EditorAssetManager.h"

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
	
	static void SerializeEntity(YAML::Emitter& out, const Entity& e)
	{
		GE_CORE_ASSERT(e.HasComponent<IDComponent>(), "Cannot serialize Entity without ID.");

		IDComponent& idc = e.GetComponent<IDComponent>();
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << idc.ID;
		if (e.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent
			auto& tc = e.GetComponent<TagComponent>();

			out << YAML::Key << "Tag" << YAML::Value << Project::GetTagByKey(tc.ID).c_str();
			out << YAML::Key << "ID" << YAML::Value << tc.ID;

			out << YAML::EndMap; // TagComponent
		}

		if (e.HasComponent<NameComponent>())
		{
			out << YAML::Key << "NameComponent";
			out << YAML::BeginMap; // NameComponent
			auto& name = e.GetComponent<NameComponent>().Name;
			GE_TRACE("UUID : {0},\n\tName : {1}", (uint64_t)idc.ID, name.c_str());

			out << YAML::Key << "Name" << YAML::Value << name;

			out << YAML::EndMap; // NameComponent
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

		if (e.HasComponent<RenderComponent>())
		{
			out << YAML::Key << "RenderComponent";
			out << YAML::BeginMap; // RenderComponent
			auto& rc = e.GetComponent<RenderComponent>();

			out << YAML::Key << "IDs" << YAML::Value;
			out << YAML::BeginSeq;
			for (uint64_t id : rc.LayerIDs)
			{
				if (Ref<Layer> layer = Application::GetLayerStack<EditorLayerStack>()->GetLayerAtIndex(id))
				{
					out << YAML::BeginMap; // LayerID
					out << YAML::Key << "Loadable" << YAML::Value << layer->IsBase();
					out << YAML::Key << "Name" << YAML::Value << Application::GetLayerStack<EditorLayerStack>()->GetLayerName(id).c_str();
					out << YAML::Key << "ID" << YAML::Value << layer->GetID();
					out << YAML::EndMap; // LayerID
				}
			}
			out << YAML::EndSeq;

			out << YAML::EndMap; // RenderComponent
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

			out << YAML::Key << "AssetHandle" << YAML::Value << component.AssetHandle;

			// Fields
			if (Scripting::ScriptExists(component.AssetHandle))
			{
				Ref<Script> script = Scripting::GetScript(component.AssetHandle);
				ScriptFieldMap& instanceFields = Scripting::GetEntityFields(e.GetComponent<IDComponent>().ID);
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

	static bool DeserializeEntity(const YAML::detail::iterator_value& eDetails, Entity& e)
	{
		// TransformComponent
		auto transformComponent = eDetails["TransformComponent"];
		if (transformComponent)
		{
			auto& nc = e.GetOrAddComponent<TransformComponent>();
			nc.Translation = transformComponent["Translation"].as<glm::vec3>();
			nc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
			nc.Scale = transformComponent["Scale"].as<glm::vec3>();
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

		// RenderComponent
		auto renderComponent = eDetails["RenderComponent"];
		if (renderComponent)
		{
			auto& rc = e.GetOrAddComponent<RenderComponent>();

			if (auto IDs = renderComponent["IDs"])
			{
				for (auto id : IDs)
				{
					bool loadableLayer = id["Loadable"].as<bool>();
					std::string layerName = id["Name"].as<std::string>();

					Ref<EditorLayerStack> els = Application::GetLayerStack<EditorLayerStack>();
					if (loadableLayer) // GE::Layer only
					{
						Ref<Layer> newLayer = CreateRef<Layer>();
						if(els && els->InsertLayer(newLayer, layerName))
							rc.AddID(newLayer->GetID());
						else if(els->LayerNameExists(layerName))
							rc.AddID(els->GetLayerIndex(layerName));
					}
					else // Any class inheriting from GE::Layer needs to be created in Client App.
					{
						if (els->LayerNameExists(layerName))
							rc.AddID(els->GetLayerIndex(layerName));
					}

				}
			}
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
			auto& src = e.GetOrAddComponent<ScriptComponent>();

			auto handle = scriptComponent["AssetHandle"];
			if(handle)
				src.AssetHandle = handle.as<uint64_t>();

			// Fields
			auto fields = scriptComponent["ScriptFields"];
			if (fields)
			{
				auto& deserializedInstanceFields = Scripting::GetEntityFields(e.GetComponent<IDComponent>().ID);

				for (auto field : fields)
				{
					std::string scriptFieldName = field["Name"].as<std::string>();
					std::string scriptFieldTypeString = field["Type"].as<std::string>();
					ScriptField::Type scriptFieldType = Scripting::StringToScriptFieldType(scriptFieldTypeString);

					ScriptField& instanceScriptField = deserializedInstanceFields[scriptFieldName];
					instanceScriptField = ScriptField(scriptFieldTypeString, scriptFieldType, nullptr);
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

#pragma region Script
	/*
	* Expected input = "namespace/className.cs"
	* Expected output = "namespace.className"
	*/
 	static const std::string GetScriptFullNameFromFilepath(const std::filesystem::path& filepath)
	{
		const std::string filePathStr = filepath.string();
		uint64_t pos = filePathStr.find('/');
		std::string nameSpace = filePathStr.substr(0, pos);
		std::string className = filePathStr.substr(pos + 1);
		return std::string(nameSpace + "." + className);
	}

	/*
	* Expected input = "namespace.className"
	* Expected output = "namespace/className.cs"
	*/
	static const std::string GetScriptFilepathFromFullName(const std::string& fullName)
	{
		uint64_t pos = fullName.find('.');
		std::string nameSpace = fullName.substr(0, pos);
		std::string className = fullName.substr(pos + 1);
		return std::string(nameSpace + "/" + className + ".cs");
	}
#pragma endregion

	AssetMetadata EditorAssetManager::s_NullMetadata = AssetMetadata();

	EditorAssetManager::EditorAssetManager(const AssetMap& assetMap /*= AssetMap()*/) : m_LoadedAssets(assetMap)
	{
		m_AssetRegistry = CreateRef<AssetRegistry>();
	}

	EditorAssetManager::~EditorAssetManager()
	{
		m_LoadedAssets.clear();
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(UUID handle)
	{
		if (!HandleExists(handle))
			return s_NullMetadata;
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
		// Special case for Scripts since they cannot be added through AssetPanel currently
		if (asset->GetType() == Asset::Type::Script && !HandleExists(handle))
		{
			AssetMetadata metadata = AssetMetadata();
			Ref<Script> script = Project::GetAssetAs<Script>(asset);
			if (script)
			{
				std::filesystem::path path = GetScriptFilepathFromFullName(script->GetFullName());
				metadata = AssetMetadata(asset->GetHandle(), path);
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
		for (const auto& [handle, metadata] : m_AssetRegistry->GetRegistry())
		{
			// if Asset isn't Scene, get from loaded. Otherwise, get runtime Scene from Project.
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

		if (!data["Scene"])
			return nullptr;

		std::string sceneName = data["Scene"].as<std::string>();
		Ref<Scene> scene = CreateRef<Scene>(metadata.Handle);
		GE_TRACE("Deserializing Scene\n\tUUID : {0}\n\tName : {1}\n\tPath : {2}", (uint64_t)metadata.Handle, sceneName.c_str(), path.string().c_str());

		auto entities = data["Entities"];
		if (entities)
		{
			GE_TRACE("Deserializing Entities");

			for (auto e : entities)
			{
				// ID Component 
				uint64_t uuid = e["Entity"].as<uint64_t>(); // UUID

				// TagComponent
				std::string tag = std::string();
				uint32_t tagID = 0;
				auto tc = e["TagComponent"];
				if (tc)
				{
					tag = tc["Tag"].as<std::string>();
					tagID = tc["ID"].as<uint32_t>();
					if(!tag.empty())
						Project::AddTag(tag, tagID);
				}

				// NameComponent
				std::string name = std::string("New Entity");
				auto nc = e["NameComponent"];
				if (nc)
					name = nc["Name"].as<std::string>();

				Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name, tagID);
				if(DeserializeEntity(e, deserializedEntity))
					GE_TRACE("UUID : {0},\n\tTag : {1}\n\tName : {2}", uuid, Project::GetTagByKey(deserializedEntity.GetComponent<TagComponent>().ID).c_str(), name.c_str());
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
		std::string filePathStr = metadata.FilePath.string();
		std::filesystem::path noExPath = filePathStr.substr(0, filePathStr.find_last_of("."));
		std::string fullNameFromPath = GetScriptFullNameFromFilepath(noExPath);
		const std::unordered_map<std::string, Ref<Script>>& scripts = Scripting::GetScripts();
		if (scripts.find(fullNameFromPath) != scripts.end())
		{
			Scripting::SetScriptHandle(fullNameFromPath, metadata.Handle);
			return scripts.at(fullNameFromPath);
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
				SerializeEntity(out, entity);
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
