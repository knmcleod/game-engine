#include "GE/GEpch.h"

#include "AssetSerializer.h"

#include "GE/Asset/Assets/Audio/Audio.h"
#include "GE/Asset/Assets/Scene/Scene.h"

#include "GE/Project/Project.h"

#include "GE/Scripting/Scripting.h"

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
		{ Asset::Type::AudioSource, AssetSerializer::DeserializeAudio		},
	};

	std::map<Asset::Type, AssetPackDeserializeFunction> AssetSerializer::s_AssetPackDeserializeFuncs =
	{
		{ Asset::Type::Scene,		AssetSerializer::DeserializeSceneFromPack		},
		{ Asset::Type::Texture2D,	AssetSerializer::DeserializeTexture2DFromPack	},
		{ Asset::Type::Font,		AssetSerializer::DeserializeFontFromPack		},
		{ Asset::Type::AudioSource, AssetSerializer::DeserializeAudioFromPack		},
	};

	std::map<Asset::Type, AssetSerializeFunction> AssetSerializer::s_AssetSerializeFuncs =
	{
		{ Asset::Type::Scene, AssetSerializer::SerializeScene }
	};

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

		std::string fontString = metadata.FilePath.string();
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
		atlasPacker.setSpacing(0.0);
		atlasPacker.setScale(scale);
		int remaining = atlasPacker.pack(msdfData->Glyphs.data(), msdfData->Glyphs.size());

		int width, height;
		atlasPacker.getDimensions(width, height);
		scale = atlasPacker.getScale();
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
				}, msdfData->Glyphs.size()).finish(atlasConfig.ThreadCount);
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
		atlasGenerator.generate(msdfData->Glyphs.data(), msdfData->Glyphs.size());
	
		msdfgen::BitmapConstRef<T, N> bitmap = atlasGenerator.atlasStorage();
		TextureConfiguration config;
		config.Height = bitmap.height;
		config.Width = bitmap.width;
		config.InternalFormat = ImageFormat::RBG8;
		config.Format = DataFormat::RGB;
		config.GenerateMips = false;
	
		Buffer dataBuffer((void*)bitmap.pixels, bitmap.height * bitmap.width * (config.InternalFormat == ImageFormat::RBG8 ? 3 : 4));
		Ref<Texture2D> texture = Texture2D::Create(config, dataBuffer);

		return texture;
	}

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

		if (audioBuffer->Size <= 0)
			return false;
			
		if (audioBuffer->BufferType == AudioBuffer::Type::Short)
		{
			Ref<ShortAudioBuffer> buffer = GetAudioBuffer<ShortAudioBuffer>(audioBuffer);
			char* data = buffer->Data;
			data = new char[audioBuffer->Size];

			stream.read(data, audioBuffer->Size);

		}
		else if (audioBuffer->BufferType == AudioBuffer::Type::Long)
		{
			Ref<LongAudioBuffer> buffer = GetAudioBuffer<LongAudioBuffer>(audioBuffer);
			for (int i = 0; i < buffer->NUM_BUFFERS; i++)
			{
				char* soundData = new char[audioBuffer->Size];
				stream.read(soundData, audioBuffer->Size);
				buffer->Data[i] = soundData;
			}
		}

		stream.close();
		return true;
	}

	bool AssetSerializer::SerializeRegistry(Ref<AssetRegistry> registry)
	{
		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : registry->GetRegistry())
			{
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
		if (!fout.good() || !fout.is_open())
		{
			GE_CORE_ERROR("Failed to Serialize Asset Registry");
			return false;
		}
		fout << out.c_str();
		fout.close();

		return true;
	}

	bool AssetSerializer::DeserializeRegistry(const std::filesystem::path& filePath, Ref<AssetRegistry> registry)
	{
		registry->m_FilePath = filePath;

		const std::filesystem::path& path = Project::GetPathToAsset(registry->m_FilePath);
		if (path.empty())
			return false;

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load asset registry file. {0}\n\t{1}", path.string(), e.what());
		}

		YAML::Node assetRegistryData = data["AssetRegistry"];
		if (!assetRegistryData)
		{
			GE_WARN("Cannot deserialize asset registry.");
			return false;
		}

		for (const auto& node : assetRegistryData)
		{
			UUID handle = node["Handle"].as<uint64_t>();
			AssetMetadata assetMetadata;
			assetMetadata.Handle = handle;
			assetMetadata.FilePath = node["FilePath"].as<std::string>();
			assetMetadata.Type = AssetUtils::AssetTypeFromString(node["Type"].as<std::string>());

			registry->AddAsset(assetMetadata);
		}

		return true;
	}

	bool AssetSerializer::SerializePack(Ref<AssetPack> pack)
	{
		// gap file

		// header, info [] = byte
		//  [4] signature "GAP"
		//  [4] Version // File Format Version
		//  [8] Build Version // Time Built

		// index, data
		//  [8] App Binary Offset // Start of
		//  [8] App Binary Size   // Size of
		//  [?] Scene Map
		//      [8] Asset Handle    : Key
		//      SceneInfo           : Value
		//          [8] Packed Offset
		//          [8] Packed Size
		//          [2] Flags
		//          [?] Asset Map
		//              [8] Asset Handle    : Key
		//              AssetInfo           : Value
		//                  [8] Packed Offset
		//                  [8] Packed Size
		//                  [2] Flags
		//                  [2] Asset Type

		std::ofstream stream(Project::GetPathToAsset(pack->m_File.Path), std::ios::out | std::ios::binary | std::ios::app);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Binary file to write.");
			return false;
		}

		// Header
		stream.write(pack->m_File.Header.HEADER, sizeof(pack->m_File.Header.HEADER));
		stream.write(reinterpret_cast<const char*>(&pack->m_File.Header.Version),
			sizeof(pack->m_File.Header.Version));
		stream.write(reinterpret_cast<const char*>(&pack->m_File.Header.BuildVersion),
			sizeof(pack->m_File.Header.BuildVersion));

		// Index
		stream.write(reinterpret_cast<const char*>(&pack->m_File.Index.Offset), sizeof(pack->m_File.Index.Offset));
		stream.write(reinterpret_cast<const char*>(&pack->m_File.Index.Size), sizeof(pack->m_File.Index.Size));
		stream.write(reinterpret_cast<const char*>(pack->m_File.Index.Scenes.size()), sizeof(pack->m_File.Index.Scenes.size()));
		for (const auto& [sceneHandle, sceneInfo] : pack->m_File.Index.Scenes)
		{
			stream.write(reinterpret_cast<const char*>(sceneHandle), sizeof(sceneHandle));
			stream.write(reinterpret_cast<const char*>(&sceneInfo.Offset), sizeof(sceneInfo.Offset));
			stream.write(reinterpret_cast<const char*>(&sceneInfo.Size), sizeof(sceneInfo.Size));
			stream.write(reinterpret_cast<const char*>(sceneInfo.Flags), sizeof(sceneInfo.Flags));
			stream.write(reinterpret_cast<const char*>(sceneInfo.Assets.size()), sizeof(sceneInfo.Assets.size()));
			for (const auto& [assetHandle, assetInfo] : sceneInfo.Assets)
			{
				stream.write(reinterpret_cast<const char*>((uint64_t)assetHandle), sizeof((uint64_t)assetHandle));
				stream.write(reinterpret_cast<const char*>(assetInfo.Offset), sizeof(assetInfo.Offset));
				stream.write(reinterpret_cast<const char*>(assetInfo.Size), sizeof(assetInfo.Size));
				stream.write(reinterpret_cast<const char*>(assetInfo.Flags), sizeof(assetInfo.Flags));
				stream.write(reinterpret_cast<const char*>(assetInfo.Type), sizeof(assetInfo.Type));

			}

		}
		stream.close();

		bool status = stream.good();
		if (status)
		{
			const auto& index = pack->m_File.Index;
			for (const auto& [sceneHandle, sceneInfo] : index.Scenes)
			{
				pack->m_HandleIndex.insert(sceneHandle);
			}
		}

		return status;
	}

	bool AssetSerializer::DeserializePack(Ref<AssetPack> pack)
	{
		pack = CreateRef<AssetPack>();

		std::ifstream stream(Project::GetPathToAsset(pack->m_File.Path), std::ios::out | std::ios::binary);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Binary file to read.");
			return false;
		}

		// Header
		stream.read((char*)&pack->m_File.Header.HEADER, sizeof(pack->m_File.Header.HEADER));
		stream.read((char*)&pack->m_File.Header.Version, sizeof(pack->m_File.Header.Version));
		stream.read((char*)&pack->m_File.Header.BuildVersion, sizeof(pack->m_File.Header.BuildVersion));

		// Index
		stream.read((char*)&pack->m_File.Index.Offset, sizeof(pack->m_File.Index.Offset));
		stream.read((char*)&pack->m_File.Index.Size, sizeof(pack->m_File.Index.Size));
		int sceneCount = 0;
		stream.read((char*)&sceneCount, sizeof(sceneCount));
		for (int i = 0; i < sceneCount; i++)
		{
			uint64_t sceneHandle = 0;
			stream.read((char*)&sceneHandle, sizeof(sceneHandle));
			AssetPack::File::SceneInfo sceneInfo;
			stream.read((char*)&sceneInfo.Offset, sizeof(sceneInfo.Offset));
			stream.read((char*)&sceneInfo.Size, sizeof(sceneInfo.Size));
			stream.read((char*)&sceneInfo.Flags, sizeof(sceneInfo.Flags));

			pack->m_File.Index.Scenes[sceneHandle] = sceneInfo;

			int assetCount = 0;
			stream.read((char*)&assetCount, sizeof(assetCount));
			for (int j = 0; j < assetCount; j++)
			{
				uint64_t assetHandle = 0;
				stream.read((char*)&assetHandle, sizeof(assetHandle));
				AssetPack::File::AssetInfo assetInfo;
				stream.read((char*)&assetInfo.Offset, sizeof(assetInfo.Offset));
				stream.read((char*)&assetInfo.Size, sizeof(assetInfo.Size));
				stream.read((char*)&assetInfo.Flags, sizeof(assetInfo.Flags));
				stream.read((char*)&assetInfo.Type, sizeof(assetInfo.Type));

				pack->m_File.Index.Scenes[sceneHandle].Assets[assetHandle] = assetInfo;

			}
		}

		stream.close();
		return stream.good();
	}

	Ref<Asset> AssetSerializer::DeserializeAsset(const AssetMetadata& metadata)
	{
		if (s_AssetDeserializeFuncs.find(metadata.Type) == s_AssetDeserializeFuncs.end())
		{
			GE_CORE_ERROR("Deserialize function not found for Type: {0}", AssetUtils::AssetTypeToString(metadata.Type));
			return nullptr;
		}
		return s_AssetDeserializeFuncs.at(metadata.Type)(metadata);
	}

	Ref<Asset> AssetSerializer::DeserializeAsset(const AssetPack::File::AssetInfo& assetInfo)
	{
		Asset::Type assetType = (Asset::Type)assetInfo.Type;
		if (s_AssetPackDeserializeFuncs.find(assetType) == s_AssetPackDeserializeFuncs.end())
		{
			GE_CORE_ERROR("Deserializeer function not found for Type: " + AssetUtils::AssetTypeToString(assetType));
			return nullptr;
		}
		return s_AssetPackDeserializeFuncs.at(assetType)(assetInfo);
	}

	bool AssetSerializer::SerializeAsset(const AssetMetadata& metadata)
	{
		if (s_AssetSerializeFuncs.find(metadata.Type) == s_AssetSerializeFuncs.end())
		{
			GE_CORE_ERROR("Serializer function not found for Type: " + AssetUtils::AssetTypeToString(metadata.Type));
			return false;
		}
		return s_AssetSerializeFuncs.at(metadata.Type)(metadata);
	}

	Ref<Asset> AssetSerializer::DeserializeScene(const AssetMetadata& metadata)
	{
		std::filesystem::path path = Project::GetPathToAsset(metadata.FilePath);
		std::ifstream stream(path);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data;
		try
		{
			data = YAML::Load(strStream.str());
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load asset registry file. {0}\n\t{1}", path.string(), e.what());
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		GE_CORE_TRACE("Deserializing Scene\n\tPath: {0}\n\tName: {1}", path.string(), sceneName.c_str());
		Ref<Scene> scene = CreateRef<Scene>(metadata.Handle, sceneName);

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

				GE_CORE_TRACE("Deserializing entity\n\tID: {0},\n\tName: {1}", uuid, name);

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

					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["Type"].as<int>());
					cc.Camera.SetFOV(cameraProps["FOV"].as<float>());
					cc.Camera.SetNearClip(cameraProps["Near"].as<float>());
					cc.Camera.SetFarClip(cameraProps["Far"].as<float>());

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
		
		return scene;
	}

	Ref<Asset> AssetSerializer::DeserializeSceneFromPack(const AssetPack::File::AssetInfo& assetInfo)
	{
		GE_CORE_ERROR("Cannot import Scene from AssetPack");
		return false;
		/*
		GE_CORE_TRACE("Deserializing Scene from Pack");
		Ref<Scene> scene = CreateRef<Scene>();

		asset = scene;
		return true;
		*/
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

		TextureConfiguration config;
		config.Height = height;
		config.Width = width;
		if (channels == 3)
		{
			config.InternalFormat = ImageFormat::RBG8;
			config.Format = DataFormat::RGB;
		}
		else if (channels == 4)
		{
			config.InternalFormat = ImageFormat::RBGA8;
			config.Format = DataFormat::RGBA;
		}
		else
			GE_CORE_WARN("Unsupported Texture2D Channels.");

		Ref<Texture2D> texture = Texture2D::Create(config, data);
		texture->p_Handle = metadata.Handle;

		data.Release();
		return texture;
	}

	Ref<Asset> AssetSerializer::DeserializeTexture2DFromPack(const AssetPack::File::AssetInfo& assetInfo)
	{
		GE_CORE_ERROR("Cannot import Texture2D from AssetPack");
		return false;
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
		Ref<Font> asset = CreateRef<Font>();
		asset->p_Handle = metadata.Handle;

		asset->m_AtlasConfig.Texture =  CreateAndCacheFontAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>
			(metadata, "FontAtlas", asset->m_AtlasConfig, asset->m_MSDFData);
		return asset;
	}

	Ref<Asset> AssetSerializer::DeserializeFontFromPack(const AssetPack::File::AssetInfo& assetInfo)
	{
		return Ref<Asset>();
	}

	Ref<Asset> AssetSerializer::DeserializeAudio(const AssetMetadata& metadata)
	{
		Ref<AudioSource> audioSource = CreateRef<AudioSource>();
		audioSource->p_Handle = metadata.Handle;

		if (LoadWav(Project::GetPathToAsset(metadata.FilePath), audioSource->m_AudioBuffer))
		{
			switch (audioSource->m_AudioBuffer->BufferType)
			{
			case AudioBuffer::Type::None:
			{
				GE_CORE_ERROR("Could not load Audio Source Buffer.");
				return audioSource;
			}
			case AudioBuffer::Type::Short:
			{
				Ref<ShortAudioBuffer> shortBuffer = audioSource->GetBuffer<ShortAudioBuffer>();
				alGenBuffers(1, &shortBuffer->Buffer);
				alBufferData(shortBuffer->Buffer, shortBuffer->Format, shortBuffer->Data, shortBuffer->Size, shortBuffer->SampleRate);

				ALenum error = alGetError();
				if (error != AL_NO_ERROR)
				{
					GE_CORE_ERROR("OpenAL Error: {0}", (char*)alGetString(error));
					if (shortBuffer->Buffer && alIsBuffer(shortBuffer->Buffer))
						alDeleteBuffers(1, &shortBuffer->Buffer);
					return 0;
				}
				break;
			}
			case AudioBuffer::Type::Long:
			{
				Ref<LongAudioBuffer> longBuffer = audioSource->GetBuffer<LongAudioBuffer>();
				alGenBuffers(longBuffer->NUM_BUFFERS, longBuffer->Buffers);

				for (std::size_t i = 0; i < longBuffer->NUM_BUFFERS; ++i)
				{
					alBufferData(longBuffer->Buffers[i], longBuffer->Format, &longBuffer->Data[i], longBuffer->Size, longBuffer->SampleRate);

					ALenum error = alGetError();
					if (error != AL_NO_ERROR)
					{
						GE_CORE_ERROR("OpenAL Error: {0}", (char*)alGetString(error));
						if (longBuffer->Buffers[i] && alIsBuffer(longBuffer->Buffers[i]))
							alDeleteBuffers(1, &longBuffer->Buffers[i]);
						return 0;
					}
				}
				break;
			}
			}
		}

		return audioSource;
	}

	Ref<Asset> AssetSerializer::DeserializeAudioFromPack(const AssetPack::File::AssetInfo& assetInfo)
	{
		return Ref<Asset>();
	}
	
	bool AssetSerializer::SerializeScene(const AssetMetadata& metadata)
	{
		std::filesystem::path path = Project::GetPathToAsset(metadata.FilePath);
		GE_CORE_TRACE("Serializing Scene\n\tPath: {}", path.string());
		Ref<Scene> scene = Project::GetAsset<Scene>(metadata.Handle);
		
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
		if (!fout.good() || !fout.is_open())
		{
			GE_CORE_ERROR("Failed to Serialize Scene Asset");
			return false;
		}
		fout << out.c_str();
		fout.close();
		return true;
	}

}