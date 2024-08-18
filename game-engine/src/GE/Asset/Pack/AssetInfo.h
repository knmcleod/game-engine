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
	* * * ~ Asset Specific Data
	* * * * ...
	*/
	struct AssetInfo
	{
		friend class AssetPack;
		friend class AssetSerializer;
	public:
		virtual bool operator==(AssetInfo other) const
		{
			if (other.Type == this->Type)
				return true;
			return false;
		}

		void InitializeData(uint64_t size)
		{
			DataBuffer = Buffer(size);
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
		* * * - ID : UUID
		* * * - Tag : Name
		* * * ~ Transform :
		* * * * - Translation :
		* * * * - Rotation :
		* * * * - Scale :
		* * * ~ Camera :
		* * * * ~ SceneCamera :
		* * * * * - Fov : float
		* * * * * - NearClip : float
		* * * * * - FarClip : float
		* * * * - Primary : bool
		* * * * - FixedAspectRatio : bool
		* * * ~ AudioSource :
		* * * * - Asset Handle :
		* * * * - Loop : bool
		* * * * - Pitch : float
		* * * * - Gain : float
		* * * ~ AudioListener :
		* * * * -
		* * * ~ SpriteRenderer :
		* * * * - Asset Handle :
		* * * * - Color : vec4
		* * * * - TilingFactor : float
		* * * ~ CircleRenderer :
		* * * * - Asset Handle :
		* * * * - Color : vec4
		* * * * - TilingFactor : float
		* * * * - Radius : float
		* * * * - Thickness : float
		* * * * - Fade : float
		* * * ~ TextRenderer :
		* * * * - Asset Handle :
		* * * * - TextColor : vec4
		* * * * - BGColor : vec4
		* * * * - KerningOffset : float
		* * * * - LineHeightOffset : float
		* * * * - Text : std::string
		* * * ~ Rigidbody2D :
		* * * * - Type : uint16_t
		* * * * - FixedRotation : bool
		* * * ~ BoxCollider2D :
		* * * * - Offset : vec2
		* * * * - Size : vec2
		* * * * - Density : float 
		* * * * - Friction : float 
		* * * * - Restitution : float 
		* * * * - RestitutionThreshold : float 
		* * * * - Show : bool 
		* * * ~ CircleCollider2D :
		* * * * - Offset : vec2
		* * * * - Radius : float
		* * * * - Density : float 
		* * * * - Friction : float 
		* * * * - Restitution : float 
		* * * * - RestitutionThreshold : float 
		* * * * - Show : bool 
		* * * ~ NativeScript : 
		* * * * - 
		* * * ~ Script :
		* * * * - ClassName : std::string
		* * * * ~ Fields :
		* * * * * - Name : std::string
		* * * * * - Type : char*
		* * * * * - Data :
		*/
		struct EntityInfo
		{
			Buffer DataBuffer = 0;
		};

		SceneInfo()
		{
			Type = 1; // See Asset::Type
		}

		SceneInfo(const AssetInfo& assetInfo) : SceneInfo()
		{
			if(assetInfo.Type == Type)
				DataBuffer = assetInfo.DataBuffer;
		}

	private:
		// Set using AssetInfo::Data
		std::map<uint64_t, AssetInfo> m_Assets = std::map<uint64_t, AssetInfo>(); // UUID, AssetInfo
		std::map<uint64_t, EntityInfo> m_Entities = std::map<uint64_t, EntityInfo>(); // UUID, EntityInfo
	};
}
