#pragma once

#include "GE/Asset/AssetMetadata.h"
#include "GE/Asset/Managers/AssetManager.h"

#include <map>

namespace GE
{
	using AssetMap = std::map<UUID, Ref<Asset>>;
	using AssetRegistry = std::map<UUID, AssetMetadata>;

	class EditorAssetManager : public AssetManager
	{
	public:
		virtual ~EditorAssetManager();

		virtual Ref<Asset> GetAsset(UUID handle) override;
		virtual bool HandleExists(UUID handle) override;
		virtual bool AssetLoaded(UUID handle) override;

		virtual bool SaveAsset(UUID handle) override;
		virtual bool RemoveAsset(UUID handle) override;

		virtual bool SerializeAssets() override;
		virtual bool DeserializeAssets() override;

		inline const AssetRegistry GetAssetRegistry() const { return m_AssetRegistry; }
		const AssetMetadata& GetMetadata(UUID handle);
		const UUID GetHandle(const AssetMetadata& metadata) const;
		Ref<Asset> GetAsset(const AssetMetadata& metadata);
		bool HandleExists(const AssetMetadata& metadata);

	private:
		AssetMap m_LoadedAssets;
		AssetRegistry m_AssetRegistry;
	};
}

