#pragma once

#include "GE/Asset/Asset.h"
#include "GE/Core/Core.h"
#include "GE/Core/UUID/UUID.h"

namespace GE
{
	class AssetManager
	{
	public:
		virtual Ref<Asset> GetAsset(UUID handle) = 0;
		virtual bool HandleExists(UUID handle) = 0;
		virtual bool AssetLoaded(UUID handle) = 0;

		virtual bool SaveAsset(UUID handle) = 0;
		virtual bool RemoveAsset(UUID handle) = 0;

		virtual bool SerializeAssets() = 0;
		virtual bool DeserializeAssets() = 0;
	};

}