#pragma once

#include "../Assets/Asset.h"
#include "../Registry/AssetRegistry.h"
#include "../Pack/AssetPack.h"

/*
* Based on CPU
*/
constexpr auto BYTE_ALIGNMENT = (sizeof(void*));

namespace GE
{
	/*
	* Returns an aligned size for dataSize
	* if dataSize is divisible by BYTE_ALIGNMENT, add the remaining size
	* else, add 0
	*/
	template<typename T>
	static inline T Aligned(T dataSize)
	{
		if (BYTE_ALIGNMENT)
		{
			
			return dataSize +
				((dataSize % BYTE_ALIGNMENT) != 0
					? BYTE_ALIGNMENT - (dataSize % BYTE_ALIGNMENT)
					: 0);
		}
		return dataSize;
	}

	// MOVE : Implement in each Asset(Scene, Texture2D, Font, Audio)
	/*
	* Converts Asset to byte array for Binary Serialization
	* Returns size of filled buffer, already Aligned
	*	
	* Assumes only std::string/char use, not std::wstring/wchar_t
	* 
	* param buffer		: pointer to buffer to be filled
	* param bufferSize	: size of buffer to be filled
	*/
	static size_t ToByteArray(void* buffer = nullptr, size_t bufferSize = 0)
	{
		size_t retSize = 0;

		UUID handle = 0;
		Asset::Type type = Asset::Type::None;
		std::string name = "Asset Name";

		// Contains Asset Handles
		// Example for Scene
		std::vector<uint64_t> assets;
		assets.emplace_back(handle);

		// Required size for whole Asset
		size_t requiredSize = 
			// int
			Aligned(sizeof(handle)) // sizeof(uint64_t)
			+ Aligned(sizeof(type))	// sizeof(uint16_t)
			// string
			+ Aligned(sizeof(size_t)) // sizeof(stringLength)
			+ Aligned(name.size() * sizeof(char)); //sizeof(string)
		
		// For vectors
		{
			requiredSize += Aligned(sizeof(size_t)); //sizeof(assetCount)
			for each (const auto& asset in assets)
			{
				//Project::GetAsset(asset)->ToByteArray()
				//requiredSize += asset->GetByteArray(); // sizeof(asset)
			}
		}

		if (buffer)
		{
			if (bufferSize >= requiredSize)
			{
				// Start at beginning of buffer
				uint8_t* destination = (uint8_t*)buffer;
				uint8_t* end = destination + requiredSize;

				// Clear requiredSize from destination
				memset(destination, 0, requiredSize);

				// Fill out buffer

				// int
				*(uint64_t*)destination = handle;
				destination += Aligned(sizeof(handle));

				*(uint16_t*)destination = (uint16_t)type;
				destination += Aligned(sizeof(type));

				// string
				size_t stringSize = name.size();
				*(size_t*)destination = stringSize;
				destination += Aligned(sizeof(stringSize));
				memcpy(destination, name.c_str(), stringSize * sizeof(char));
				destination += Aligned(name.size() * sizeof(char));
				
				// For vectors
				{
					size_t assetCount = assets.size();
					*(size_t*)destination = assetCount;
					destination += Aligned(sizeof(assetCount));

					for each (const auto & asset in assets)
					{
						//size_t assetSize = asset->GetByteArray(destination, end - destination);
						/*if (assetSize)
						{
							destination += assetSize;
						}
						else
						{
							GE_CORE_ASSERT(false, "Buffer overflow.");
						}
						*/
					}

				}

				if (destination - (uint8_t*)buffer == requiredSize)
				{
					retSize = requiredSize;
				}
				else
				{
					GE_CORE_ASSERT(false, "Buffer overflow.");
				}
			}
			else
			{
				GE_CORE_ERROR("Required size is larger than given buffer size.");
			}
		}
		else
		{
			retSize = requiredSize;
		}

		return retSize;
	}

	using AssetDeserializeFunction = std::function<Ref<Asset>(const AssetMetadata&)>;
	using AssetPackDeserializeFunction = std::function<Ref<Asset>(const AssetInfo&)>;

	using AssetSerializeFunction = std::function<bool(Ref<Asset>, const AssetMetadata&)>;
	using AssetPackSerializeFunction = std::function<bool(Ref<Asset>, const AssetInfo&)>;

	class AssetSerializer
	{
	public:
		static bool SerializeRegistry(Ref<AssetRegistry> registry);
		static bool DeserializeRegistry(Ref<AssetRegistry> registry);

		static bool SerializePack(Ref<AssetPack> registry);
		static bool DeserializePack(Ref<AssetPack> registry);

		static Ref<Asset> DeserializeAsset(const AssetMetadata& metadata);
		static Ref<Asset> DeserializeAsset(const AssetInfo& assetInfo);

		static bool SerializeAsset(Ref<Asset> asset, const AssetMetadata& metadata);
		/*
		* Returns true if asset is successfully copied to assetInfo
		* Uses asset->GetByteArray() to set assetInfo.Data & assetInfo.Size
		*/
		static bool SerializeAsset(Ref<Asset> asset, const AssetInfo& assetInfo);

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
		static bool SerializeSceneToPack(Ref<Asset> asset, const AssetInfo& assetInfo);

		static std::map<Asset::Type, AssetDeserializeFunction> s_AssetDeserializeFuncs;
		static std::map<Asset::Type, AssetPackDeserializeFunction> s_AssetPackDeserializeFuncs;

		static std::map<Asset::Type, AssetSerializeFunction> s_AssetSerializeFuncs;
		static std::map<Asset::Type, AssetPackSerializeFunction> s_AssetPackSerializeFuncs;
	};
}
