#pragma once

#include <cstdint>

namespace GE
{
	/*
	* Contains
	* - Type : Asset Type - Texture2D, Font, AudioClip.
	* ~ DataBuffer : Allocated only for Serialize/Deserialize
	* * - Size
	* * ~ Data : Set in corresponding AssetSerializer::SerializeAsset(asset, assetInfo)
	* * * - Handle
	* * * - Type
	* * * - Name
	* * * ~ Asset Specific Data
	* * * * ...
	*/
	struct AssetInfo
	{
		friend class AssetPack;
		friend class AssetSerializer;
	public:
		/*
		* Compares type of Asset
		*/
		virtual bool operator==(AssetInfo other) const
		{
			if (other.Type == this->Type)
				return true;
			return false;
		}

		AssetInfo() = default;

		AssetInfo(uint16_t type) : Type(type)
		{

		}

		AssetInfo(const AssetInfo& assetInfo) : Type(assetInfo.Type)
		{
			InitializeData(assetInfo.DataBuffer.GetSize(), assetInfo.DataBuffer.As<uint8_t>());
		}

		void InitializeData(uint64_t size, const uint8_t* data = nullptr)
		{
			DataBuffer = Buffer(data, size);
		}

		uint16_t Type = 0;	// Asset Type: Scene, Texture2D, Font, AudioClip

		Buffer DataBuffer = 0;
	};

	/*
	* Special case for Scene Assets
	* Comparable to .scene + .gar file
	* Contains
	* - Type : Asset Type - Scene
	* - Assets : map<UUID, AssetInfo>
	* - Entities : map<UUID, EntityInfo>
	* ~ DataBuffer : Allocated only for Serialize/Deserialize
	* * - Size : Size of Data
	* * ~ Data : Set in corresponding AssetSerializer::SerializeAsset(asset, assetInfo)
	* * * - Handle
	* * * - Type
	* * * - Name
	* * * ~ Scene Data
	* * * * - Assets : map<UUID, AssetInfo>
	* * * * - Entities : map<UUID, EntityInfo>
	*/
	struct SceneInfo : public AssetInfo
	{
		friend class AssetPack;
		friend class AssetSerializer;
	public:
		/*
		* Contains
		* - Size : Packed Data Size
		* ~ Data : Packed Data
		* * ~ Components : Components on Entity
		* * * - ID : UUID/uint64_t
		* * * - Tag : std::string
		* * * ~ Transform :
		* * * * - Translation : vec3<float>
		* * * * - Rotation : vec3<float>
		* * * * - Scale : vec3<float>
		* * * ~ Camera :
		* * * * ~ SceneCamera :
		* * * * * - Fov : float
		* * * * * - NearClip : float
		* * * * * - FarClip : float
		* * * * - Primary : bool
		* * * * - FixedAspectRatio : bool
		* * * ~ AudioSource :
		* * * * - Asset Handle : UUID/uint64_t
		* * * * - Loop : bool
		* * * * - Pitch : float
		* * * * - Gain : float
		* * * ~ AudioListener :
		* * * * -
		* * * ~ SpriteRenderer :
		* * * * - Asset Handle : UUID/uint64_t
		* * * * - TilingFactor : float
		* * * * - Color : vec4<float>
		* * * ~ CircleRenderer :
		* * * * - Asset Handle :
		* * * * - TilingFactor : float
		* * * * - Radius : float
		* * * * - Thickness : float
		* * * * - Fade : float
		* * * * - Color : vec4<float>
		* * * ~ TextRenderer :
		* * * * - Asset Handle :
		* * * * - KerningOffset : float
		* * * * - LineHeightOffset : float
		* * * * - Text : std::string
		* * * * - TextColor : vec4<float>
		* * * * - BGColor : vec4<float>
		* * * ~ Rigidbody2D :
		* * * * - Type : uint16_t
		* * * * - FixedRotation : bool
		* * * ~ BoxCollider2D :
		* * * * - Show : bool 
		* * * * - Density : float 
		* * * * - Friction : float 
		* * * * - Restitution : float 
		* * * * - RestitutionThreshold : float
		* * * * - Offset : vec2<float>
		* * * * - Size : vec2<float>
		* * * ~ CircleCollider2D :
		* * * * - Show : bool 
		* * * * - Radius : float
		* * * * - Density : float 
		* * * * - Friction : float 
		* * * * - Restitution : float 
		* * * * - RestitutionThreshold : float 
		* * * * - Offset : vec2<float>
		* * * ~ NativeScript : 
		* * * * - 
		* * * ~ Script :
		* * * * - ClassName : std::string
		*/
		struct EntityInfo
		{
			EntityInfo() = default;

			EntityInfo(const EntityInfo& entityInfo)
			{
				InitializeData(entityInfo.DataBuffer.GetSize(), entityInfo.DataBuffer.As<uint8_t>());
			}

			void InitializeData(uint64_t size, const uint8_t* data = nullptr)
			{
				DataBuffer = Buffer(data, size);
			}

			Buffer DataBuffer = 0;
		};

		SceneInfo()
		{
			Type = 1; // See Asset::Type
		}

		SceneInfo(const SceneInfo& sceneInfo) : AssetInfo(sceneInfo)
		{
			Type = 1; // See Asset::Type

			for (auto& [uuid, assetInfo] : sceneInfo.m_Assets)
			{
				m_Assets[uuid] = AssetInfo(assetInfo);
			}
			for (auto& [uuid, entityInfo] : sceneInfo.m_Entities)
			{
				m_Entities[uuid] = EntityInfo(entityInfo);
			}
		}

		void ClearAllData()
		{
			DataBuffer.Release();
			DataBuffer = 0;

			for (auto& [uuid, assetInfo] : m_Assets)
			{
				assetInfo.DataBuffer.Release();
				assetInfo.DataBuffer = 0;
			}

			for (auto& [uuid, entityInfo] : m_Entities)
			{
				entityInfo.DataBuffer.Release();
				entityInfo.DataBuffer = 0;
			}
		}

	private:
		// Set using AssetInfo::Data
		std::map<uint64_t, AssetInfo> m_Assets = std::map<uint64_t, AssetInfo>(); // UUID, AssetInfo
		std::map<uint64_t, EntityInfo> m_Entities = std::map<uint64_t, EntityInfo>(); // UUID, EntityInfo
	};
}
