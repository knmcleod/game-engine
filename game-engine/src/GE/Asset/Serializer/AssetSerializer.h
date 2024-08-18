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

	class AssetSerializer
	{
	public:
		static bool SerializeRegistry(Ref<AssetRegistry> registry);
		static bool DeserializeRegistry(Ref<AssetRegistry> registry);

		static bool SerializePack(Ref<AssetPack> pack);
		static bool DeserializePack(Ref<AssetPack> pack);

		static Ref<Asset> DeserializeAsset(const AssetMetadata& metadata);
		/*
		* 
		*/
		static Ref<Asset> DeserializeAsset(const AssetInfo& assetInfo);

		static bool SerializeAsset(Ref<Asset> asset, const AssetMetadata& metadata);

		/*
		* 
		*/
		static bool SerializeAsset(Ref<Asset> asset, AssetInfo& assetInfo);

	private:
		static Ref<Asset> DeserializeScene(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeSceneFromPack(const AssetInfo& assetInfo);
		
		static Ref<Asset> DeserializeTexture2D(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeTexture2DFromPack(const AssetInfo& assetInfo);

		static Ref<Asset> DeserializeFont(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeFontFromPack(const AssetInfo& assetInfo);

		static Ref<Asset> DeserializeAudio(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeAudioFromPack(const AssetInfo& assetInfo);

		static bool SerializeScene(Ref<Asset> asset, const AssetMetadata& metadata);
		static bool SerializeSceneForPack(Ref<Asset> asset, AssetInfo& assetInfo);

		static bool SerializeTexture2DForPack(Ref<Asset> asset, AssetInfo& assetInfo);
		static bool SerializeFontForPack(Ref<Asset> asset, AssetInfo& assetInfo);
		static bool SerializeAudioForPack(Ref<Asset> asset, AssetInfo& assetInfo);

		static std::map<Asset::Type, AssetDeserializeFunction> s_AssetDeserializeFuncs;
		static std::map<Asset::Type, AssetPackDeserializeFunction> s_AssetPackDeserializeFuncs;

		static std::map<Asset::Type, AssetSerializeFunction> s_AssetSerializeFuncs;
		static std::map<Asset::Type, AssetPackSerializeFunction> s_AssetPackSerializeFuncs;
	};
}
