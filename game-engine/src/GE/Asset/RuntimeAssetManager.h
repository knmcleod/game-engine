#pragma once

#include "AssetManager.h"

#include "Pack/AssetPack.h"

namespace GE
{
	class RuntimeAssetManager : public AssetManager
	{
	public:
		RuntimeAssetManager();
		~RuntimeAssetManager() override;

		virtual Ref<Asset> GetAsset(UUID handle) override;

		virtual bool HandleExists(UUID handle) override;
		virtual bool AssetLoaded(UUID handle) override;

		virtual bool AddAsset(UUID handle) override;
		virtual bool RemoveAsset(UUID handle) override;

		virtual bool SerializeAssets() override;
		virtual bool DeserializeAssets() override;
	private:
		Ref<AssetPack> m_AssetPack;
		AssetMap m_LoadedAssets;
	};

}