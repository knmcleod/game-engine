#pragma once

#include "../Assets/Asset.h"
#include "../Registry/AssetRegistry.h"
#include "../Pack/AssetPack.h"

namespace GE
{
	using AssetDeserializeFunction = std::function<Ref<Asset>(const AssetMetadata&)>;
	using AssetPackDeserializeFunction = std::function<Ref<Asset>(const AssetInfo&)>;

	using AssetSerializeFunction = std::function<bool(Ref<Asset>, const AssetMetadata&)>;
	using AssetPackSerializeFunction = std::function<bool(Ref<Asset>, AssetInfo&)>;

	class Entity;

	class AssetSerializer
	{
	public:
		/*
		* Uses AssetRegistry FilePath to open .gar(Game Asset Registry) file & fill AssetRegistry registry<UUID, AssetMetadata>
		* Does not load Assets
		* Use Deserialize(const AssetMetadata&) to load.
		*/
		static bool DeserializeRegistry(Ref<AssetRegistry> registry);
		/*
		* Uses AssetPack FilePath to open .gap(Game Asset Pack) file & fills AssetPack + Active AssetManager
		* Adds Scene Asset if whole scene is successfully deserialized, meaning:
		* 1. Scene Size & Data was read
		* 2. Data was parsed into SceneInfo, filling all appropiate AssetInfo & EntityInfo Buffers.
		* * - See DeserializeAsset(AssetInfo&) for Scene Asset Type
		* 3. Those Assets & Entities were also deserialized whole.
		* * - See Type matched DeserializeAsset(AssetInfo&) & DeserializeEntityFromPack(const EntityInfo&, Entity&)
		*/
		static bool DeserializePack(Ref<AssetPack> pack);

		/*
		* Uses loaded assets in EditorAssetManager to fill AssetRegistry & write to .gar(Game Asset Registry) file
		*/
		static bool SerializeRegistry(Ref<AssetRegistry> registry);
		/*
		* Uses loaded assets in RuntimeAssetManager to fill AssetPack & write to .gap(Game Asset Pack) file
		*/
		static bool SerializePack(Ref<AssetPack> pack);

		static Ref<Asset> DeserializeAsset(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeAsset(const AssetInfo& assetInfo);

		static bool SerializeAsset(Ref<Asset> asset, const AssetMetadata& metadata);
		static bool SerializeAsset(Ref<Asset> asset, AssetInfo& assetInfo);

	private:
		static Ref<Asset> DeserializeScene(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeTexture2D(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeFont(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeAudio(const AssetMetadata& metadata);

		static bool SerializeScene(Ref<Asset> asset, const AssetMetadata& metadata);

		static Ref<Asset> DeserializeSceneFromPack(const AssetInfo& assetInfo);
		static bool DeserializeEntity(const SceneInfo::EntityInfo& eInfo, Entity& e);

		static Ref<Asset> DeserializeTexture2DFromPack(const AssetInfo& assetInfo);
		static Ref<Asset> DeserializeFontFromPack(const AssetInfo& assetInfo);
		static Ref<Asset> DeserializeAudioFromPack(const AssetInfo& assetInfo);

		static bool SerializeSceneForPack(Ref<Asset> asset, AssetInfo& assetInfo);
		static bool SerializeEntity(SceneInfo::EntityInfo& eInfo, const Entity& e);

		static bool SerializeTexture2DForPack(Ref<Asset> asset, AssetInfo& assetInfo);
		static bool SerializeFontForPack(Ref<Asset> asset, AssetInfo& assetInfo);
		static bool SerializeAudioForPack(Ref<Asset> asset, AssetInfo& assetInfo);
	private:
		static std::map<Asset::Type, AssetDeserializeFunction> s_AssetDeserializeFuncs;
		static std::map<Asset::Type, AssetPackDeserializeFunction> s_AssetPackDeserializeFuncs;

		static std::map<Asset::Type, AssetSerializeFunction> s_AssetSerializeFuncs;
		static std::map<Asset::Type, AssetPackSerializeFunction> s_AssetPackSerializeFuncs;

		enum class ComponentType
		{
			None = 0,
			ID = 1,
			Tag = 2,
			Transform = 3,
			Camera = 4,
			AudioSource = 5,
			AudioListener = 6,
			SpriteRenderer = 7,
			CircleRenderer = 8,
			TextRenderer = 9,
			Rigidbody2D = 10,
			BoxCollider2D = 11,
			CircleCollider2D = 12,
			NativeScript = 13,
			Script = 14,
		};
	};
}
