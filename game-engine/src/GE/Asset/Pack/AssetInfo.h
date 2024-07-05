#pragma once

#include <cstdint>

namespace GE
{
	/*
	* Contains
	* - Size : Packed Data Size
	* ~ Data : Packed Data
	* * - Type : Asset Type - Texture2D, Font, AudioClip.
	*/
	class AssetInfo
	{
		friend class AssetPack;
		friend class AssetSerializer;
	public:
		bool operator==(AssetInfo other) const
		{
			if (other.p_Type == this->p_Type && other.p_Data == this->p_Data)
				return true;
		}

	protected:
		uint16_t p_Type = 0;	// Asset Type: Texture2D, Font, AudioClip

		uint64_t p_Size = 0;	// Packed Data Size
		uint8_t* p_Data = nullptr;
	};

	/*
	* Special case for Scene Assets
	* Comparable to .scene + .gar file
	* Contains
	* - Size : Total Scene size, excluding Offset
	* ~ Data : Packed Data
	* * - Type : Asset Type - Scene
	* * - Assets : map<UUID, AssetInfo>
	* * - Entities : map<UUID, EntityInfo>
	*/
	class SceneInfo : public AssetInfo
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
			uint64_t Size = 0;	// Packed Data Size
			uint8_t* Data = nullptr;
		};
	private:

		// Set using Data
		std::map<uint64_t, AssetInfo> m_Assets; // UUID, AssetInfo
		std::map<uint64_t, EntityInfo> m_Entities; // UUID, EntityInfo
	};
}
