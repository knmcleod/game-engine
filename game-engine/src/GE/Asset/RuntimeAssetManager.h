#pragma once

#include "AssetManager.h"

#include "Pack/AssetPack.h"

#include <map>

namespace GE
{
	// Foward declaration
	class Entity;
	class Scene;

	class RuntimeAssetManager : public AssetManager
	{
	public:
		enum class ComponentType
		{
			None = 0,
			ID,
			Tag,
			Name,
			Active,
			Relationship,
			Transform,
			AudioSource,
			AudioListener,
			Render,
			Camera,
			SpriteRenderer,
			CircleRenderer,
			TextRenderer,
			GUICanvas,
			GUILayout,
			// TODO : GUIMaskComponent
			GUIImage,
			GUIButton,
			GUIInputField,
			GUISlider,
			GUICheckbox,
			// TODO : GUIScrollRectComponent & GUIScrollbarComponent
			Rigidbody2D,
			BoxCollider2D,
			CircleCollider2D,
			NativeScript,
			Script
		};

		RuntimeAssetManager(const AssetMap& assetMap = AssetMap());
		virtual ~RuntimeAssetManager() override;

		virtual void InvalidateAssets() override;

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
		* Uses loaded assets to fill AssetPack & write to .gap(Game Asset Pack) file
		*/
		virtual bool SerializeAssets() override;
		/*
		* Uses AssetPack FilePath to open .gap(Game Asset Pack) file & fills AssetPack + RuntimeAssetManager
		* Adds Scene Asset if whole scene is successfully deserialized, meaning:
		* 1. Scene Size & Data was read
		* 2. Data was parsed into SceneInfo, filling all appropiate AssetInfo & EntityInfo Buffers.
		* * - See DeserializeAsset(AssetInfo&) for Scene Asset Type
		* 3. Those Assets & Entities were also deserialized whole.
		* * - See Type matched DeserializeAsset(AssetInfo&) & DeserializeEntityFromPack(const EntityInfo&, Entity&)
		*/
		virtual bool DeserializeAssets() override;

		Ref<AssetPack> GetPack() { return m_AssetPack; }

	private:
		bool SerializeAsset(Ref<Asset> asset, AssetInfo& assetInfo);
		Ref<Asset> DeserializeAsset(const AssetInfo& assetInfo);

		bool SerializeScene(Ref<Asset> asset, AssetInfo& assetInfo);
		bool SerializeEntity(Ref<Scene>, SceneInfo::EntityInfo& eInfo, const Entity& e);
		bool SerializeTexture2D(Ref<Asset> asset, AssetInfo& assetInfo);
		bool SerializeFont(Ref<Asset> asset, AssetInfo& assetInfo);
		bool SerializeAudio(Ref<Asset> asset, AssetInfo& assetInfo);
		bool SerializeScript(Ref<Asset> asset, AssetInfo& assetInfo);

		Ref<Asset> DeserializeScene(const AssetInfo& assetInfo);
		bool DeserializeEntity(Ref<Scene> scene, const SceneInfo::EntityInfo& eInfo, Entity& e);
		Ref<Asset> DeserializeTexture2D(const AssetInfo& assetInfo);
		Ref<Asset> DeserializeFont(const AssetInfo& assetInfo);
		Ref<Asset> DeserializeAudio(const AssetInfo& assetInfo);
		Ref<Asset> DeserializeScript(const AssetInfo& assetInfo);

	private:
		Ref<AssetPack> m_AssetPack = nullptr;
		// <uint64_t, Ref<Asset>>
		AssetMap m_LoadedAssets;
	};

}