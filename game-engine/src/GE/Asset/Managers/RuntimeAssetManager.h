#pragma once

#include "GE/Asset/Managers/AssetManager.h"

#include <map>

namespace GE
{
	using AssetMap = std::map<UUID, Ref<Asset>>;

	class RuntimeAssetManager : public AssetManager
	{
	public:
		virtual Ref<Asset> GetAsset(UUID handle) override;
		virtual bool HandleExists(UUID handle) override;
		virtual bool AssetLoaded(UUID handle) override;

		virtual bool SaveAsset(UUID handle) override;
		virtual bool RemoveAsset(UUID handle) override;

		virtual bool SerializeAssets() override;
		virtual bool DeserializeAssets() override;
	private:
		AssetMap m_LoadedAssets;
	};

}