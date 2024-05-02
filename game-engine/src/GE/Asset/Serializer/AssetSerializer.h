#pragma once

#include "../Asset.h"
#include "../Registry/AssetRegistry.h"
#include "../Pack/AssetPack.h"

namespace GE
{
	using AssetDeserializeFunction = std::function<Ref<Asset>(const AssetMetadata&)>;
	using AssetPackDeserializeFunction = std::function<Ref<Asset>(const AssetPack::File::AssetInfo&)>;

	using AssetSerializeFunction = std::function<bool(const AssetMetadata&)>;

	class AssetSerializer
	{
	public:
		static bool SerializeRegistry(Ref<AssetRegistry> registry);
		static bool DeserializeRegistry(const std::filesystem::path& filePath, Ref<AssetRegistry> registry);

		static bool SerializePack(Ref<AssetPack> registry);
		static bool DeserializePack(Ref<AssetPack> registry);

		static Ref<Asset> DeserializeAsset(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeAsset(const AssetPack::File::AssetInfo& assetInfo);

		static bool SerializeAsset(const AssetMetadata& metadata);
		static bool SerializeAsset(const AssetPack::File::AssetInfo& assetInfo);
		
	private:
		static Ref<Asset> DeserializeScene(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeSceneFromPack(const AssetPack::File::AssetInfo& assetInfo);
		
		static Ref<Asset> DeserializeTexture2D(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeTexture2DFromPack(const AssetPack::File::AssetInfo& assetInfo);

		static Ref<Asset> DeserializeFont(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeFontFromPack(const AssetPack::File::AssetInfo& assetInfo);

		static Ref<Asset> DeserializeAudio(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeAudioFromPack(const AssetPack::File::AssetInfo& assetInfo);

		static Ref<Asset> DeserializeLongAudio(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeLongAudioFromPack(const AssetPack::File::AssetInfo& assetInfo);

		static bool SerializeScene(const AssetMetadata& metadata);

		static std::map<Asset::Type, AssetDeserializeFunction> s_AssetDeserializeFuncs;
		static std::map<Asset::Type, AssetPackDeserializeFunction> s_AssetPackDeserializeFuncs;

		static std::map<Asset::Type, AssetSerializeFunction> s_AssetSerializeFuncs;
	};
}
