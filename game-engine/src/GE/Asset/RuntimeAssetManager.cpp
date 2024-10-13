#include "GE/GEpch.h"

#include "RuntimeAssetManager.h"

#include "GE/Asset/Assets/Audio/Audio.h"
#include "GE/Asset/Assets/Font/Font.h"

#include "GE/Core/Memory/Buffer.h"

#include "GE/Project/Project.h"

#include "GE/Scripting/Scripting.h"

namespace GE
{

#pragma region Font
	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> func>
	static Ref<Texture2D> LoadFontAtlas(Buffer data, const uint32_t& width, const uint32_t& height, Font::AtlasConfig& atlasConfig, Ref<Font::MSDFData> msdfData)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
		{
			GE_CORE_ERROR("Failed to load Font Freetype Handle");
			return 0;
		}

		msdfgen::FontHandle* font = msdfgen::loadFontData(ft, data.As<uint8_t>(), (int)data.GetSize());
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

		Texture::Config config = Texture::Config(width, height, 0, false);
		config.InternalFormat = Texture::ImageFormat::RGB8;
		config.Format = Texture::DataFormat::RGB;

		Buffer dataBuffer(data.As<void>(),
			config.Height * config.Width * (config.InternalFormat == Texture::ImageFormat::RGB8 ? 3 : 4));
		Ref<Texture2D> texture = Texture2D::Create(0, config, dataBuffer);
		dataBuffer.Release();

		return texture;
	}

#pragma endregion

	RuntimeAssetManager::RuntimeAssetManager(const AssetMap& assetMap /*= AssetMap()*/) : m_LoadedAssets(assetMap)
	{
		m_AssetPack = CreateRef<AssetPack>();
	}

	RuntimeAssetManager::~RuntimeAssetManager()
	{
		m_LoadedAssets.clear();
		m_AssetPack->ClearAllFileData();
	}

	Ref<Asset> RuntimeAssetManager::GetAsset(UUID handle)
	{
		Ref<Asset> asset = nullptr;
		if (!HandleExists(handle))
		{
			GE_CORE_ERROR("Runtime Asset Handle does not exist.");
			return asset;
		}

		if (AssetLoaded(handle))
		{
			asset = m_LoadedAssets.at(handle);
		}
		else
		{
			const AssetInfo& assetInfo = m_AssetPack->GetAssetInfo(handle);
			asset = DeserializeAsset(assetInfo);
			if (asset)
				m_LoadedAssets.at(handle) = asset;
		}

		return asset;
	}

	const AssetMap& RuntimeAssetManager::GetLoadedAssets()
	{
		return m_LoadedAssets;
	}

	bool RuntimeAssetManager::HandleExists(UUID handle)
	{
		return m_AssetPack->HandleExists(handle);
	}

	bool RuntimeAssetManager::AssetLoaded(UUID handle)
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}
	
	bool RuntimeAssetManager::AddAsset(Ref<Asset> asset)
	{
		if (!asset || AssetLoaded(asset->GetHandle()))
		{
			GE_CORE_WARN("Cannot add Asset to Loaded. Asset already in Loaded.");
			return false;
		}

		UUID handle = asset->GetHandle();
		m_LoadedAssets.emplace(handle, asset);
		m_AssetPack->AddAsset(m_LoadedAssets.at(handle), AssetInfo((uint16_t)m_LoadedAssets.at(handle)->GetType()));
		return true;
	}

	bool RuntimeAssetManager::RemoveAsset(UUID handle)
	{
		if (!HandleExists(handle) || !AssetLoaded(handle))
			return false;

		return m_AssetPack->RemoveAsset(m_LoadedAssets.at(handle));
	}

	bool RuntimeAssetManager::SerializeAssets()
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

		AssetPack::File& packFile = m_AssetPack->m_File;
		std::filesystem::path path = Project::GetPathToAsset(packFile.Path);
		GE_CORE_INFO("AssetRegistry Serialization Started.\n\tFilePath : {0}", path.string().c_str());

		std::ofstream stream(path, std::ios::app | std::ios::binary);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Pack file to write.");
			return false;
		}

		// Header
		{
			stream.write(reinterpret_cast<const char*>(packFile.FileHeader.HEADER), sizeof(packFile.FileHeader.HEADER));
			stream.write(reinterpret_cast<const char*>(&packFile.FileHeader.Version), sizeof(packFile.FileHeader.Version));
		}

		// Index
		{
			for (const auto& [uuid, asset] : Project::GetAssetManager<RuntimeAssetManager>()->GetLoadedAssets())
			{
				if (asset->GetType() == Asset::Type::Scene)
				{
					if (SerializeAsset(asset, packFile.Index.Scenes[uuid]))
					{
						// Scene and its children have been populated into pack->m_File.Index.Scenes.at(uuid)
						// Scene.Size should represent the total scene size and be aligned
						// Add to Index.Size for each Scene Asset
						packFile.Index.Size += packFile.Index.Scenes.at(uuid).DataBuffer.GetSize();
					}
				}
			}

			// Index.Size contains
			// - Scene Count
			// - All Scenes
			packFile.Index.Size += GetAligned(sizeof(packFile.Index.Scenes.size()));
			stream.write(reinterpret_cast<const char*>(&packFile.Index.Size), sizeof(packFile.Index.Size));

			uint64_t allScenesCount = packFile.Index.Scenes.size();
			stream.write(reinterpret_cast<const char*>(&allScenesCount), sizeof(uint64_t));
			for (const auto& [sceneHandle, sceneInfo] : packFile.Index.Scenes)
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

	bool RuntimeAssetManager::DeserializeAssets()
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

		AssetPack::File& packFile = m_AssetPack->m_File;
		std::filesystem::path path = Project::GetPathToAsset(packFile.Path);
		GE_CORE_INFO("Asset Pack Deserialization Started.\n\tFilePath : {0}", path.string().c_str());

		std::ifstream stream(path, std::ios::app | std::ios::binary);
		if (!stream)
		{
			GE_CORE_ERROR("Could not open Asset Pack file to read.");
			return false;
		}

		// Header
		{
			stream.read((char*)&packFile.FileHeader.HEADER, sizeof(packFile.FileHeader.HEADER));
			stream.read((char*)&packFile.FileHeader.Version, sizeof(packFile.FileHeader.Version));
		}

		// Index
		{
			stream.read((char*)&packFile.Index.Size, sizeof(packFile.Index.Size));
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
					if (Project::GetAssetManager()->AddAsset(sceneAsset) && m_AssetPack->AddAsset(sceneAsset, sceneInfo))
					{
						for (const auto& [uuid, childAssetInfo] : sceneInfo.Assets)
						{
							if (Ref<Asset> childAsset = DeserializeAsset(childAssetInfo))
							{
								Project::GetAssetManager()->AddAsset(childAsset);
							}
						}

						Ref<Scene> scene = Project::GetAssetAs<Scene>(sceneAsset);
						for (const auto& [uuid, entityInfo] : sceneInfo.Entities)
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

#pragma region Asset Info Serialization

	bool RuntimeAssetManager::SerializeAsset(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		switch (asset->GetType())
		{
		case Asset::Type::Scene:
			return SerializeScene(asset, assetInfo);
			break;
		case Asset::Type::Texture2D:
			return SerializeTexture2D(asset, assetInfo);
			break;
		case Asset::Type::Font:
			return SerializeFont(asset, assetInfo);
			break;
		case Asset::Type::Audio:
			return SerializeAudio(asset, assetInfo);
		case Asset::Type::Script:
			return SerializeScript(asset, assetInfo);
			break;
		default:
			GE_CORE_ERROR("Serialize asset info function not found for Type: " + AssetUtils::AssetTypeToString(asset->GetType()));
			break;
		}
		return false;
	}

	Ref<Asset> RuntimeAssetManager::DeserializeAsset(const AssetInfo& assetInfo)
	{
		Asset::Type assetType = (Asset::Type)assetInfo.Type;
		switch (assetType)
		{
		case Asset::Type::Scene:
			return DeserializeScene(assetInfo);
			break;
		case Asset::Type::Texture2D:
			return DeserializeTexture2D(assetInfo);
			break;
		case Asset::Type::Font:
			return DeserializeFont(assetInfo);
			break;
		case Asset::Type::Audio:
			return DeserializeAudio(assetInfo);
			break;
		case Asset::Type::Script:
			return DeserializeScript(assetInfo);
			break;
		default:
			GE_CORE_ERROR("Deserialize function not found for Type: " + AssetUtils::AssetTypeToString(assetType));
			break;
		}
		return nullptr;
	}

	bool RuntimeAssetManager::SerializeScene(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		// ScenePackFormat
		//      [84 + ?] SceneInfo  : Value, corresponding Key handled in SerializePack
		//          [8] Packed Size : Size of whole Scene
		//			[76 + ?] Data
		//				[8] Handle
		//				[2] Type
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
			const UUID& sceneHandle = scene->GetHandle();
			const Scene::Config& sceneConfig = scene->GetConfig();
			requiredSize += GetAligned(sizeof(scene->GetHandle())) // sizeof(uint64_t)
				+ GetAligned(sizeof(scene->GetType()))	// sizeof(uint16_t)
				+ GetAligned(sizeof(sceneConfig.StepFrames)); // sizeof(uint64_t)

			// For Assets
			{
				AssetMap assetMap = Project::GetAssetManager<RuntimeAssetManager>()->GetLoadedAssets();
				requiredSize += GetAligned(sizeof(assetMap.size())); //sizeof(assetCount)
				for (const auto& [uuid, asset] : assetMap)
				{
					if (uuid == sceneHandle || asset->GetType() == Asset::Type::Scene)
						continue;

					if (SerializeAsset(asset, sceneInfo.Assets[uuid]))
					{
						// += SizeofData + Data
						requiredSize += GetAlignedOfArray<uint8_t>(sceneInfo.Assets.at(uuid).DataBuffer.GetSize());
					}
				}
			}

			// For Entities
			{
				requiredSize += GetAligned(sizeof(uint64_t)); //sizeof(eCount)
				{
					// Every Entity needs an ID
					std::vector<Entity> entities = scene->GetAllEntitiesWith<IDComponent>();
					for (Entity entity : entities)
					{
						UUID uuid = entity.GetComponent<IDComponent>().ID;
						if (SerializeEntity(sceneInfo.Entities[uuid], entity))
						{
							// += SizeofData + Data
							requiredSize += GetAlignedOfArray<uint8_t>(sceneInfo.Entities.at(uuid).DataBuffer.GetSize());
						}

					}

					entities.clear();
					entities = std::vector<Entity>();

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
						const Scene::Config& sceneConfig = scene->GetConfig();
						WriteAligned<uint64_t>(destination, scene->GetHandle());
						WriteAligned<uint16_t>(destination, (uint16_t)scene->GetType());

						WriteAligned<uint64_t>(destination, sceneConfig.StepFrames);

						// For Assets
						{
							WriteAligned<uint64_t>(destination, sceneInfo.Assets.size());

							for (const auto& [uuid, assetInfo] : sceneInfo.Assets)
							{
								//  asset size & data
								const uint8_t* data = assetInfo.DataBuffer.As<uint8_t>();
								WriteAlignedArray<uint8_t>(destination, data, assetInfo.DataBuffer.GetSize());

							}

						}

						// For Entities
						{
							WriteAligned<uint64_t>(destination, sceneInfo.Entities.size());

							for (const auto& [uuid, eInfo] : sceneInfo.Entities)
							{
								// e size & data
								const uint8_t* data = eInfo.DataBuffer.As<uint8_t>();
								WriteAlignedArray<uint8_t>(destination, data, eInfo.DataBuffer.GetSize());
							}
						}
					}

					if (destination - assetInfo.DataBuffer.As<uint8_t>() == requiredSize)
					{
						GE_CORE_INFO("RuntimeAssetManager::SerializeScene Successful.");
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

	bool RuntimeAssetManager::SerializeEntity(SceneInfo::EntityInfo& eInfo, const Entity& e)
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
				requiredSize += GetAligned(sizeof(uint16_t)) + GetAligned(sizeof(tc.ID));
			}

			// Name Component 
			if (e.HasComponent<NameComponent>())
			{
				NameComponent nc = e.GetComponent<NameComponent>();
				// Component identifier + Name String
				requiredSize += GetAligned(sizeof(uint16_t)) + GetAlignedOfArray<char>(nc.Name.size());
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

			// Render
			if (e.HasComponent<RenderComponent>())
			{
				auto& rc = e.GetComponent<RenderComponent>();
				// Component identifier
				requiredSize += GetAligned(sizeof(uint16_t));

				requiredSize += GetAligned<uint64_t>(rc.LayerIDs.size() * sizeof(uint64_t));
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
				requiredSize += GetAligned(sizeof(sc.AssetHandle));

				// Field Name, Type & Data
				const ScriptFieldMap& fields = Scripting::GetEntityFields(e.GetComponent<IDComponent>().ID);
				requiredSize += GetAligned(sizeof(uint64_t)); // size of Fields
				for (const auto& [name, field] : fields)
				{
					const char* fieldName = name.c_str();
					requiredSize += GetAlignedOfArray<char>(name.size()) // Field Name
						+ GetAligned(sizeof(uint16_t));					// Field Type

					// Field Data
					switch (field.GetType())
					{
					case ScriptField::Type::Char:
						requiredSize += GetAligned(sizeof(char));
						break;
					case ScriptField::Type::Int:
						requiredSize += GetAligned(sizeof(int));
						break;
					case ScriptField::Type::UInt:
						requiredSize += GetAligned(sizeof(uint32_t));
						break;
					case ScriptField::Type::Float:
						requiredSize += GetAligned(sizeof(float));
						break;
					case ScriptField::Type::Byte:
						requiredSize += GetAligned(sizeof(int8_t));
						break;
					case ScriptField::Type::Bool:
						requiredSize += GetAligned(sizeof(bool));
						break;
					case ScriptField::Type::Vector2:
					{
						requiredSize += GetAlignedOfVec2<float>();
					}
					break;
					case ScriptField::Type::Vector3:
					{
						requiredSize += GetAlignedOfVec3<float>();
					}
					break;
					case ScriptField::Type::Vector4:
					{
						requiredSize += GetAlignedOfVec4<float>();
					}
					break;
					case ScriptField::Type::Entity:
						requiredSize += GetAligned(sizeof(uint64_t));
						break;
					}
				}
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

							WriteAligned<uint32_t>(destination, tc.ID);

						}

						// Name
						if (e.HasComponent<NameComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::Name;
							WriteAligned(destination, currentType);

							NameComponent nc = e.GetComponent<NameComponent>();

							const char* tagCStr = nc.Name.c_str();
							WriteAlignedArray<char>(destination, tagCStr, nc.Name.size());

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

						// Render
						if (e.HasComponent<RenderComponent>())
						{
							uint16_t currentType = (uint16_t)ComponentType::Render;
							WriteAligned(destination, currentType);

							auto& rc = e.GetComponent<RenderComponent>();
							for (uint64_t id : rc.LayerIDs)
							{
								WriteAligned<uint64_t>(destination, id);
							}
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

							WriteAligned<uint64_t>(destination, sc.AssetHandle);

							// Field Name, Type & Data
							const ScriptFieldMap& fields = Scripting::GetEntityFields(e.GetComponent<IDComponent>().ID);
							WriteAligned<uint64_t>(destination, fields.size());
							for (const auto& [name, field] : fields)
							{
								const char* fieldName = name.c_str();
								WriteAlignedArray<char>(destination, fieldName, name.size());
								const uint32_t type = (uint32_t)field.GetType();
								WriteAligned<uint32_t>(destination, type);
								switch (field.GetType())
								{
								case ScriptField::Type::Char:
									WriteAligned<char>(destination, field.GetValue<char>());
									break;
								case ScriptField::Type::Int:
									WriteAligned<int>(destination, field.GetValue<int>());
									break;
								case ScriptField::Type::UInt:
									WriteAligned<uint32_t>(destination, field.GetValue<uint32_t>());
									break;
								case ScriptField::Type::Float:
									WriteAligned<float>(destination, field.GetValue<float>());
									break;
								case ScriptField::Type::Byte:
									WriteAligned<uint8_t>(destination, field.GetValue<uint8_t>());
									break;
								case ScriptField::Type::Bool:
									WriteAligned<bool>(destination, field.GetValue<bool>());
									break;
								case ScriptField::Type::Vector2:
								{
									const glm::vec2 data = field.GetValue<glm::vec2>();
									WriteAlignedVec2<float>(destination, data.x, data.y);
								}
									break;
								case ScriptField::Type::Vector3:
								{
									const glm::vec3 data = field.GetValue<glm::vec3>();
									WriteAlignedVec3<float>(destination, data.x, data.y, data.z);
								}
									break;
								case ScriptField::Type::Vector4:
								{
									const glm::vec4 data = field.GetValue<glm::vec4>();
									WriteAlignedVec4<float>(destination, data.x, data.y, data.z, data.w);
								}
									break;
								case ScriptField::Type::Entity:
									WriteAligned<uint64_t>(destination, field.GetValue<uint64_t>());
									break;
								}
							}
						}

					}
				}

				if (destination - eInfo.DataBuffer.As<uint8_t>() == requiredSize)
				{
					GE_CORE_INFO("RuntimeAssetManager::SerializeEntity(EntityInfo&, const Entity&) Successful.");
					return true;
				}
				else
				{
					GE_CORE_ASSERT(false, "RuntimeAssetManager::SerializeEntity(EntityInfo&, const Entity&) Buffer Overflow.");
				}
			}
			else
			{
				GE_CORE_ERROR("RuntimeAssetManager::SerializeEntity(EntityInfo&, const Entity&)\n\tRequired size is larger than given buffer size.");
			}
		}

		return false;
	}

	bool RuntimeAssetManager::SerializeTexture2D(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		Ref<Texture2D> texture = Project::GetAssetAs<Texture2D>(asset);
		assetInfo.Type = 2; // See Asset::Type

		uint64_t requiredSize = 0;

		// Size
		{
			requiredSize = GetAligned(sizeof(uint64_t)) // Handle
				+ GetAligned(sizeof(uint16_t));	// Type

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
					WriteAligned<uint64_t>(destination, texture->GetHandle());
					WriteAligned<uint16_t>(destination, (uint16_t)texture->GetType());

					const Texture::Config& textureConfig = texture->GetConfig();
					// Texture::Config
					{
						WriteAligned<uint32_t>(destination, textureConfig.Width);

						WriteAligned<uint32_t>(destination, textureConfig.Height);

						WriteAligned<uint16_t>(destination, (uint16_t)textureConfig.InternalFormat);

						WriteAligned<uint16_t>(destination, (uint16_t)textureConfig.Format);

						WriteAligned<bool>(destination, textureConfig.GenerateMips);

					}

					// Texture Buffer Size & Data
					const uint8_t* data = textureConfig.TextureBuffer.As<uint8_t>();
					WriteAlignedArray<uint8_t>(destination, data, (uint64_t)textureConfig.TextureBuffer.GetSize());

				}

				if (destination - assetInfo.DataBuffer.As<uint8_t>() == requiredSize)
				{
					GE_CORE_INFO("RuntimeAssetManager::SerializeTexture2D() Successful.");
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

	bool RuntimeAssetManager::SerializeFont(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		/*
		* - Handle
		* - Type
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
				+ GetAligned(sizeof(uint16_t));									// Type

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

				WriteAligned<uint64_t>(destination, font->GetHandle());
				WriteAligned<uint16_t>(destination, (uint16_t)font->GetType());

				const Font::AtlasConfig& fontConfig = font->GetAtlasConfig();

				// Config
				{
					const Texture::Config& textureConfig = atlasTexture->GetConfig();
					WriteAligned<uint32_t>(destination, textureConfig.Width);
					WriteAligned<uint32_t>(destination, textureConfig.Height);
					WriteAligned<float>(destination, fontConfig.Scale);
					WriteAligned<uint64_t>(destination, fontConfig.Seed);
					WriteAligned<uint32_t>(destination, fontConfig.ThreadCount);
					WriteAligned<bool>(destination, fontConfig.ExpensiveColoring);

					// FontTexture Size & Data
					const uint8_t* data = textureConfig.TextureBuffer.As<uint8_t>();
					WriteAlignedArray<uint8_t>(destination, data, atlasTexture->GetConfig().TextureBuffer.GetSize());
				}

				if (destination - assetInfo.DataBuffer.As<uint8_t>() == requiredSize)
				{
					GE_CORE_INFO("RuntimeAssetManager::SerializeFont() Successful.");
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

	bool RuntimeAssetManager::SerializeAudio(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		/*
		* - Handle
		* - Type
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
				+ GetAligned(sizeof(uint16_t));	// Type

			// Config
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
					WriteAligned<uint64_t>(destination, audio->GetHandle());
					WriteAligned<uint16_t>(destination, (uint16_t)audio->GetType());

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
					GE_CORE_INFO("RuntimeAssetManager::SerializeAudio() Successful.");
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

	bool RuntimeAssetManager::SerializeScript(Ref<Asset> asset, AssetInfo& assetInfo)
	{
		/*
		* - Handle : uin64_t
		* - Type : uint16_t
		* - Name : std::string "namespace.className"
		*/

		Ref<Script> script = Project::GetAssetAs<Script>(asset);
		assetInfo.Type = 5; // See Asset::Type::Audio
		uint64_t requiredSize = 0;

		const std::string& scriptFullName = script->GetFullName();
		// Size
		{
			requiredSize = GetAligned(sizeof(uint64_t)) // Handle
				+ GetAligned(sizeof(uint16_t))	// Type
				+ GetAligned(sizeof(size_t)) // sizeof(stringLength)
				+ GetAligned(scriptFullName.size() * sizeof(char)); //sizeof(string)

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
					WriteAligned<uint64_t>(destination, script->GetHandle());
					WriteAligned<uint16_t>(destination, (uint16_t)script->GetType());

					const char* cStr = scriptFullName.c_str();
					WriteAlignedArray<char>(destination, cStr, (uint64_t)scriptFullName.size());

				}

				if (destination - assetInfo.DataBuffer.As<uint8_t>() == requiredSize)
				{
					GE_CORE_INFO("RuntimeAssetManager::SerializeScript() Successful.");
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

		return true;
	}

	Ref<Asset> RuntimeAssetManager::DeserializeScene(const AssetInfo& assetInfo)
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
				GE_CORE_ASSERT(false, "RuntimeAssetManager::DeserializeScene(AssetInfo&) Buffer Overflow");

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

				((SceneInfo&)assetInfo).Assets[uuid].Type = type;
				((SceneInfo&)assetInfo).Assets.at(uuid).InitializeData(cadBuffer.GetSize(), cadBuffer.As<uint8_t>());

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
			if (!ReadAlignedArray<uint8_t>(source, end, data, size))
				return nullptr;

			Buffer eDataBuffer = Buffer(data, size);
			delete[] data;

			// Overflow check
			if (source > end)
				GE_CORE_ASSERT(false, "RuntimeAssetManager::DeserializeScene(AssetInfo&) Buffer Overflow");

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

					scene->CreateEntityWithUUID(uuid, "Deserialized Entity", 0);
					((SceneInfo&)assetInfo).Entities[uuid].InitializeData(eDataBuffer.GetSize(), eDataBuffer.As<uint8_t>());
				}
			}
			eDataBuffer.Release();
		}

		GE_CORE_INFO("RuntimeAssetManager::DeserializeScene(AssetInfo&) Successful");
		return scene;
	}

	bool RuntimeAssetManager::DeserializeEntity(const SceneInfo::EntityInfo& eInfo, Entity& e)
	{
		if (eInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Entity from AssetPack.\n\tEntityInfo has no Data");
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
					uint32_t tag = 0;
					if (ReadAligned(source, end, tag))
						e.GetOrAddComponent<TagComponent>().ID = tag;
				}
				break;
				case 3: // Name
				{
					uint64_t nameStringSize = 0;
					if (!ReadAligned(source, end, nameStringSize))
						break;
					char* tagCStr = new char[nameStringSize];
					if (ReadAlignedArray<char>(source, end, tagCStr, nameStringSize))
						e.GetOrAddComponent<NameComponent>().Name.assign(tagCStr, nameStringSize);
					delete[](tagCStr);
				}
				break;
				case 4: // Transform
				{
					TransformComponent nc = e.GetOrAddComponent<TransformComponent>();
					if (!ReadAlignedVec3<float>(source, end, nc.Translation.x, nc.Translation.y, nc.Translation.z))
					{
						GE_CORE_ERROR("Failed to read Entity TransformComponent Translation.");
					}

					if (!ReadAlignedVec3<float>(source, end, nc.Rotation.x, nc.Rotation.y, nc.Rotation.z))
					{
						GE_CORE_ERROR("Failed to read Entity TransformComponent Rotation.");
					}

					if (!ReadAlignedVec3<float>(source, end, nc.Scale.x, nc.Scale.y, nc.Scale.z))
					{
						GE_CORE_ERROR("Failed to read Entity TransformComponent Scale.");
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
				case 7: // Render
				{
					RenderComponent rc = e.GetOrAddComponent<RenderComponent>();
					
					uint64_t* ids = nullptr;
					size_t size = 0;
					if (ReadAlignedArray<uint64_t>(source, end, ids, size))
					{
						for (uint64_t i = 0; i < size; i++)
						{
							rc.AddID(ids[i]);
						}
					}
					else
					{
						GE_CORE_ERROR("Failed to read RenderComponent Tag IDs.");
					}
				}
				break;
				case 8: // Camera
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
				case 9: // SpriteRenderer
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
				case 10: // CircleRenderer
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
				case 11: // TextRenderer
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
				case 12: // Rigidbody2D
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
				case 13: // BoxCollider2D
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
				case 14: // CircleCollider2D
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
				case 15: // NativeScript
				{
					NativeScriptComponent nsc = e.GetOrAddComponent<NativeScriptComponent>();

				}
				break;
				case 16: // Script
				{
					ScriptComponent sc = e.GetOrAddComponent<ScriptComponent>();

					uint64_t scriptHandle = 0;
					if (!ReadAligned<uint64_t>(source, end, scriptHandle))
						break;

					// Field Name, Type & Data
					uint64_t fieldsSize = 0;
					if (!ReadAligned(source, end, fieldsSize))
						return false;
					for (int i = 0; i < fieldsSize; i++)
					{
						uint64_t fieldNameSize = 0;
						if (!ReadAligned(source, end, fieldNameSize))
							return false;
						char* fieldNameCStr = new char[fieldNameSize];
						std::string fieldName;
						if (ReadAlignedArray(source, end, fieldNameCStr, fieldNameSize))
							fieldName.assign(fieldNameCStr, fieldNameSize);
						delete[](fieldNameCStr);

						uint32_t fieldType = 0;
						if (!ReadAligned(source, end, fieldType))
							return false;

						ScriptFieldMap& fields = Scripting::GetEntityFields(e.GetComponent<IDComponent>().ID);
						ScriptField& instanceField = fields[fieldName];
						switch ((ScriptField::Type)fieldType)
						{
						case ScriptField::Type::Char:
						{
							char c = 0;
							if (!ReadAligned(source, end, c))
								return false;
							instanceField.SetValue<char>(c);
						}
							break;
						case ScriptField::Type::Int:
						{
							int data = 0;
							if (!ReadAligned(source, end, data))
								return false;
							instanceField.SetValue<int>(data);
						}
							break;
						case ScriptField::Type::UInt:
						{
							uint32_t data = 0;
							if (!ReadAligned(source, end, data))
								return false;
							instanceField.SetValue<uint32_t>(data);
						}
							break;
						case ScriptField::Type::Float:
						{
							float data = 0;
							if (!ReadAligned(source, end, data))
								return false;
							instanceField.SetValue<float>(data);
						}
							break;
						case ScriptField::Type::Byte:
						{
							uint8_t data = 0;
							if (!ReadAligned(source, end, data))
								return false;
							instanceField.SetValue<uint8_t>(data);
						}
							break;
						case ScriptField::Type::Bool:
						{
							bool data = 0;
							if (!ReadAligned(source, end, data))
								return false;
							instanceField.SetValue<bool>(data);
						}
							break;
						case ScriptField::Type::Vector2:
						{
							float x, y = 0;
							if (!ReadAlignedVec2<float>(source, end, x, y))
								return false;
							glm::vec2 data = { x, y };
							instanceField.SetValue<glm::vec2>(data);
						}
						break;
						case ScriptField::Type::Vector3:
						{
							float x, y, z = 0;
							if (!ReadAlignedVec3<float>(source, end, x, y, z))
								return false;
							glm::vec3 data = { x, y, z };
							instanceField.SetValue<glm::vec3>(data);
						}
						break;
						case ScriptField::Type::Vector4:
						{
							float x, y, z, w  = 0;
							if (!ReadAlignedVec4<float>(source, end, x, y, z, w))
								return false;
							glm::vec4 data = { x, y, z, w };
							instanceField.SetValue<glm::vec4>(data);
						}
						break;
						case ScriptField::Type::Entity:
						{
							uint64_t data = 0;
							if (!ReadAligned(source, end, data))
								return false;
							instanceField.SetValue<uint64_t>(data);
						}
							break;
						}
					}
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
					GE_CORE_ASSERT(false, "RuntimeAssetManager::DeserializeEntity(const EntityInfo&, Entity&) Overflow");
					return false;
				}
			}
			else
			{
				GE_CORE_ERROR("Failed to read Entity's ComponentType.");
				return false;
			}
		}

		GE_CORE_INFO("RuntimeAssetManager::DeserializeEntity(const EntityInfo&, Entity&) Successful");
		return true;
	}

	Ref<Asset> RuntimeAssetManager::DeserializeTexture2D(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Texture2D from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		// Read Data & Assign
		const uint8_t* source = assetInfo.DataBuffer.As<uint8_t>();
		const uint8_t* end = source + assetInfo.DataBuffer.GetSize();

		// Handle
		uint64_t handle = 0;
		if (!ReadAligned<uint64_t>(source, end, handle))
			return nullptr;

		// Type has already been read, skip 
		source += GetAligned(sizeof(uint16_t));

		// Config
		uint32_t width = 0;
		if (!ReadAligned<uint32_t>(source, end, width))
			return nullptr; 
		uint32_t height = 0;
		if (!ReadAligned<uint32_t>(source, end, height))
			return nullptr;

		uint16_t internalFormat = 0;
		if (!ReadAligned<uint16_t>(source, end, internalFormat))
			return nullptr;

		uint16_t format = 0;
		if (!ReadAligned<uint16_t>(source, end, format))
			return nullptr;

		bool mips = false;
		if (!ReadAligned<bool>(source, end, mips))
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

		Texture::Config config = Texture::Config(width, height, 0, mips);
		config.InternalFormat = (Texture::ImageFormat)internalFormat;
		config.Format = (Texture::DataFormat)format;

		// Overflow check
		if (source > end)
			GE_CORE_ASSERT(false, "RuntimeAssetManager::DeserializeTexture2D(AssetInfo&) Buffer Overflow");

		// Set
		Ref<Texture2D> texture = Texture2D::Create(handle, config, textureBuffer);
		textureBuffer.Release();
		GE_CORE_INFO("RuntimeAssetManager::DeserializeTexture2D(AssetInfo&) Successful");
		return texture;
	}

	Ref<Asset> RuntimeAssetManager::DeserializeFont(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Font from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		/*
		* - Handle
		* - Type
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
			GE_CORE_ASSERT(false, "RuntimeAssetManager::DeserializeFont2D(AssetInfo&) Buffer Overflow");

		// Create & Assign Data
		Ref<Font::MSDFData> msdfData = CreateRef<Font::MSDFData>();
		config.Texture = LoadFontAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(textBuffer, width, height, config, msdfData);
		Ref<Font> font = CreateRef<Font>(handle, config, msdfData);

		textBuffer.Release();

		return font;
	}

	Ref<Asset> RuntimeAssetManager::DeserializeAudio(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Audio from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		/*
		* - Handle
		* - Type
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

		// Config
		bool loop = false;
		if (!ReadAligned<bool>(source, end, loop))
			return nullptr;
		float pitch = 1.0f;
		if (!ReadAligned<float>(source, end, pitch))
			return nullptr;
		float gain = 1.0f;
		if (!ReadAligned<float>(source, end, gain))
			return nullptr;

		uint8_t channels = 0;
		if (!ReadAligned<uint8_t>(source, end, channels))
			return nullptr;
		uint32_t sampleRate = 0;
		if (!ReadAligned<uint32_t>(source, end, sampleRate))
			return nullptr;
		uint8_t bps = 0;
		if (!ReadAligned<uint8_t>(source, end, bps))
			return nullptr;
		uint32_t format = 0;
		if (!ReadAligned<uint32_t>(source, end, format))
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
			GE_CORE_ASSERT(false, "RuntimeAssetManager::DeserializeAudio(AssetInfo&) Buffer Overflow");

		Ref<Audio> audio = Audio::Create(handle, Audio::Config(channels, sampleRate, bps, buffer));
		buffer.Release();
		GE_CORE_INFO("RuntimeAssetManager::DeserializeAudio(AssetInfo&) Successful");
		return audio;
	}

	Ref<Asset> RuntimeAssetManager::DeserializeScript(const AssetInfo& assetInfo)
	{
		if (assetInfo.DataBuffer)
		{
			GE_CORE_ERROR("Cannot import Script from AssetPack.\n\tAssetInfo has no Data");
			return nullptr;
		}

		/*
		* - Handle : uint64_t
		* - Type : uint16_t
		* - Name : std::string, "namespace.className"
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

		// Script FullName
		uint64_t sizeOfFullName = 0;
		if (ReadAligned(source, end, sizeOfFullName))
			return nullptr;
		char* fullName = new char[sizeOfFullName];
		if (!ReadAlignedArray<char>(source, end, fullName, sizeOfFullName))
			return nullptr;

		std::string fullnameStr = std::string(fullName);
		delete[] fullName;
		// Overflow check
		if (source > end)
			GE_CORE_ASSERT(false, "RuntimeAssetManager::DeserializeScript(AssetInfo&) Buffer Overflow");

		// Expected namespace.className
		Ref<Script> script = Scripting::GetScript(fullnameStr);
		GE_CORE_INFO("RuntimeAssetManager::DeserializeScript(AssetInfo&) Successful");
		return script;
	}

#pragma endregion

}