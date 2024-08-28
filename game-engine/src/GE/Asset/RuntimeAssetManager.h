#pragma once

#include "AssetManager.h"

#include "Pack/AssetPack.h"

namespace GE
{
	class RuntimeAssetManager : public AssetManager
	{
	public:
		RuntimeAssetManager(const AssetMap& assetMap = AssetMap());
		virtual ~RuntimeAssetManager() override;
		/*
		* Returns loaded asset from Loaded Asset Map or nullptr
		*	if Asset Pack loads asset, add to Loaded and return asset
		*	if handle does not exist or cannot be loaded, returns nullptr asset
		*/
		virtual Ref<Asset> GetAsset(UUID handle) override;

		virtual const AssetMap& GetLoadedAssets() override;
		/*
		* Checks if asset handle exists in Asset Pack
		* Asset may not be loaded, Use AssetLoaded(handle)
		*/
		virtual bool HandleExists(UUID handle) override;
		/*
		* Checks if asset handle exists in Loaded Asset Map
		* Loaded asset handle exists in Asset Pack
		*/
		virtual bool AssetLoaded(UUID handle) override;

		/*
		* Adds loaded asset by handle if exists
		* returns true if successfully added to Asset Pack
		*/
		virtual bool AddAsset(UUID handle) override;
		/*
		* Adds loaded asset by Ref
		* returns true if successfully added to loaded Assets
		*/
		virtual bool AddAsset(Ref<Asset> asset) override;
		/*
		* Removes loaded asset by handle if exists
		* returns true if successfully removed from Asset Pack
		*/
		virtual bool RemoveAsset(UUID handle) override;

		/*
		* Serializes all assets in Asset Pack using AssetSerializer::SerializePack(pack)
		*/
		virtual bool SerializeAssets() override;
		/*
		* De-serializes all assets into Asset Pack using AssetSerializer::DeserializePack(pack)
		*/
		virtual bool DeserializeAssets() override;

		Ref<AssetPack> GetPack() { return m_AssetPack; }

	private:
		Ref<AssetPack> m_AssetPack;
		AssetMap m_LoadedAssets;
	};

}