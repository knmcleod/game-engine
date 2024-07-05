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
		{ Asset::Type::Scene, AssetSerializer::SerializeSceneToPack }
	};
	
	static void SerializeEntity(YAML::Emitter& out, Entity entity)
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

#pragma region Audio

	template<typename T, typename U>
	static Ref<T> GetAudioBuffer(Ref<U> audioBuffer)
	{
		return static_ref_cast<T, U>(audioBuffer);
	}

	static std::int32_t convert_to_int(char* audioClip, std::size_t len)
	{
		std::int32_t a = 0;
		std::memcpy(&a, audioClip, len);
		return a;
	}

	static  bool LoadWavFile(std::ifstream& file, std::uint8_t& channels, std::int32_t& sampleRate, std::uint8_t& bitsPerSample, uint32_t& size)
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
		if (!LoadWavFile(stream, audioBuffer->Channels, audioBuffer->SampleRate, audioBuffer->BPS, audioBuffer->Size))
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
			uint8_t* soundData = new uint8_t[audioBuffer->Size];
			stream.read((char*)soundData, audioBuffer->Size);
			audioBuffer->Data += *soundData;
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

	// TODO:
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

		std::ofstream stream(Project::GetPathToAsset(pack->m_File.Path), std::ios::out | std::ios::binary | std::ios::app);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Pack file to write.");
			return false;
		}

		// MOVE: For Assets & Entities, use inside SerializeScene
		{
			size_t testSize = ToByteArray();
			if (testSize != 0)
			{
				//Reserve Memory
				uint8_t* mem = new uint8_t[testSize];

				if (ToByteArray(mem, testSize) == testSize)
				{
					GE_CORE_TRACE("Serialized Asset Pack");
				}
				else
				{
					GE_CORE_ASSERT(false, "");
				}

				delete[] mem;
				mem = nullptr;
			}
			else
			{
				GE_CORE_ASSERT(false, "");
			}
		}

		// Header
		{
			stream.write(pack->m_File.FileHeader.HEADER, sizeof(pack->m_File.FileHeader.HEADER));
			stream.write(reinterpret_cast<const char*>(&pack->m_File.FileHeader.Version),
				sizeof(pack->m_File.FileHeader.Version));

			pack->m_File.FileHeader.BuildVersion = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			stream.write(reinterpret_cast<const char*>(&pack->m_File.FileHeader.BuildVersion),
				sizeof(pack->m_File.FileHeader.BuildVersion));
		}
			
		// Index
		{
			// Set Index.Offset based on Header.Size
			pack->m_File.Index.Offset = pack->GetHeaderSize();
			stream.write(reinterpret_cast<const char*>(&pack->m_File.Index.Offset), sizeof(pack->m_File.Index.Offset));

			for (const auto& [uuid, asset] : Project::GetAssetManager()->GetLoadedAssets())
			{
				if (asset->GetType() == Asset::Type::Scene)
				{
					if (SerializeAsset(asset, pack->m_File.Index.Scenes.at(uuid)))
					{
						// Scene and its children have been populated into pack->m_File.Index.Scenes.at(uuid)
						// Scene.Size should represent the total scene size and be aligned
						// Add to Index.Size for each Scene Asset
						pack->m_File.Index.Size += pack->m_File.Index.Scenes.at(uuid).p_Size;
					}
				}
			}

			pack->m_File.Index.Size += Aligned(sizeof(pack->m_File.Index.Scenes.size()));
			stream.write(reinterpret_cast<const char*>(&pack->m_File.Index.Size), sizeof(pack->m_File.Index.Size));
			stream.write(reinterpret_cast<const char*>(pack->m_File.Index.Scenes.size()), sizeof(pack->m_File.Index.Scenes.size()));
			for (const auto& [sceneHandle, sceneInfo] : pack->m_File.Index.Scenes)
			{
				// Write all data
				stream.write(reinterpret_cast<const char*>(sceneHandle), sizeof(sceneHandle));

				stream.write(reinterpret_cast<const char*>(&sceneInfo.p_Size), sizeof(sceneInfo.p_Size));
				stream.write(reinterpret_cast<const char*>(sceneInfo.p_Data), sceneInfo.p_Size);

			}
		}
		
		stream.close();

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

		std::ifstream stream(Project::GetPathToAsset(pack->m_File.Path), std::ios::out | std::ios::binary);
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
			int sceneCount = 0;
			stream.read((char*)&sceneCount, sizeof(sceneCount));
			for (int i = 0; i < sceneCount; i++)
			{
				uint64_t sceneHandle = 0;
				stream.read((char*)&sceneHandle, sizeof(sceneHandle));
				SceneInfo sceneInfo;
				stream.read((char*)&sceneInfo.p_Size, sizeof(sceneInfo.p_Size));
				stream.read((char*)&sceneInfo.p_Data, sceneInfo.p_Size);

				if (Ref<Asset> sceneAsset = DeserializeAsset(sceneInfo))
				{
					Project::GetAssetManager()->AddAsset(sceneAsset);
					pack->AddAsset(sceneAsset, &sceneInfo);

					for (const auto& [uuid, childAsset] : sceneInfo.m_Assets)
					{
						if (Ref<Asset> asset = DeserializeAsset(childAsset))
						{
							Project::GetAssetManager()->AddAsset(asset);
							pack->AddAsset(asset, &childAsset);
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
		Asset::Type assetType = (Asset::Type)assetInfo.p_Type;
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

	bool AssetSerializer::SerializeAsset(Ref<Asset> asset, const AssetInfo& assetInfo)
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
				uint64_t uuid = entity["Entity"].as<uint64_t>(); // ID Component UUID

				// TagComponent
				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				GE_CORE_TRACE("Deserializing entity\n\tUUID : {0},\n\tName : {1}", uuid, name);

				Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name);

				// TransformComponent
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = scene->GetOrAddComponent<TransformComponent>(deserializedEntity);
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				// CameraComponent
				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = scene->GetOrAddComponent<CameraComponent>(deserializedEntity);
					auto& cameraProps = cameraComponent["Camera"];
					
					cc.ActiveCamera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["Type"].as<int>());
					cc.ActiveCamera.SetFOV(cameraProps["FOV"].as<float>());
					cc.ActiveCamera.SetNearClip(cameraProps["Near"].as<float>());
					cc.ActiveCamera.SetFarClip(cameraProps["Far"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				// AudioSourceComponent
				auto audioSourceComponent = entity["AudioSourceComponent"];
				if (audioSourceComponent)
				{
					auto& cc = scene->GetOrAddComponent<AudioSourceComponent>(deserializedEntity);
					cc.AssetHandle = audioSourceComponent["AssetHandle"].as<UUID>();
					cc.Gain = audioSourceComponent["Gain"].as<float>();
					cc.Pitch = audioSourceComponent["Pitch"].as<float>();
					cc.Loop = audioSourceComponent["Loop"].as<bool>();
				}

				// AudioListenerComponent
				auto audioListenerComponent = entity["AudioListenerComponent"];
				if (audioListenerComponent)
				{
					auto& cc = scene->GetOrAddComponent<AudioListenerComponent>(deserializedEntity);

				}

				// SpriteRendererComponent
				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = scene->GetOrAddComponent<SpriteRendererComponent>(deserializedEntity);
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
				auto circleRendererComponent = entity["CircleRendererComponent"];
				if (circleRendererComponent)
				{
					auto& src = scene->GetOrAddComponent<CircleRendererComponent>(deserializedEntity);
					src.Color = circleRendererComponent["Color"].as<glm::vec4>();
					src.Radius = circleRendererComponent["Radius"].as<float>();
					src.Thickness = circleRendererComponent["Thickness"].as<float>();
					src.Fade = circleRendererComponent["Fade"].as<float>();
				}

				// TextRendererComponent
				auto textRendererComponent = entity["TextRendererComponent"];
				if (textRendererComponent)
				{
					auto& src = scene->GetOrAddComponent<TextRendererComponent>(deserializedEntity);
					src.TextColor = textRendererComponent["TextColor"].as<glm::vec4>();
					src.BGColor = textRendererComponent["BGColor"].as<glm::vec4>();

					src.KerningOffset = textRendererComponent["KerningOffset"].as<float>();
					src.LineHeightOffset = textRendererComponent["LineHeightOffset"].as<float>();
					src.Text = textRendererComponent["Text"].as<std::string>();

					src.AssetHandle = textRendererComponent["AssetHandle"].as<UUID>();
				}

				// NativeScriptComponent
				auto nativeScriptComponent = entity["NativeScriptComponent"];
				if (nativeScriptComponent)
				{
					auto& src = scene->GetOrAddComponent<NativeScriptComponent>(deserializedEntity);
				}

				// ScriptComponent
				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					Scripting::SetScene(scene.get());

					auto& src = scene->GetOrAddComponent<ScriptComponent>(deserializedEntity);
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
				auto rigidBody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidBody2DComponent)
				{
					auto& src = scene->GetOrAddComponent<Rigidbody2DComponent>(deserializedEntity);
					src.FixedRotation = rigidBody2DComponent["FixedRotation"].as<bool>();
					src.Type = ComponentUtils::GetRigidBody2DTypeFromString(rigidBody2DComponent["Type"].as<std::string>());
				}

				// BoxCollider2DComponent
				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& src = scene->GetOrAddComponent<BoxCollider2DComponent>(deserializedEntity);
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
					auto& src = scene->GetOrAddComponent<CircleCollider2DComponent>(deserializedEntity);
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
		
		scene->p_Status = Asset::Status::Ready;
		return scene;
	}

	// TODO:
	Ref<Asset> AssetSerializer::DeserializeSceneFromPack(const AssetInfo& assetInfo)
	{
		GE_CORE_ERROR("Cannot import Scene from AssetPack");
		return nullptr;
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
					&audioSource->m_AudioBuffer->Data[i], audioSource->m_AudioBuffer->Size,
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

	bool AssetSerializer::SerializeSceneToPack(Ref<Asset> asset, const AssetInfo& assetInfo)
	{
		//      [68 + ?] SceneInfo  : Value, corressponding Key handled in SerializePack
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
		
		Ref<Scene> scene = Project::GetAssetAs<Scene>(asset);
		SceneInfo* sceneInfo = Project::GetAssetManager<RuntimeAssetManager>()->GetAssetInfo<SceneInfo>(assetInfo);
		
		sceneInfo->p_Size = scene->GetByteArray(); // Get requiredSize
		if (sceneInfo->p_Size != 0)
		{
			//Reserve Memory using requiredSize
			sceneInfo->p_Data = new uint8_t[sceneInfo->p_Size];

			// Set & Verify data
			if (scene->GetByteArray(sceneInfo->p_Data, sceneInfo->p_Size) == sceneInfo->p_Size)
			{
				GE_CORE_TRACE("Serialized Scene to Pack.");
				return true;
			}

		}
		else
		{
			GE_CORE_ASSERT(false, "");
		}

		return false;

	}

}