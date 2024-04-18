#pragma once

#include "AssetPackUtils.h"

#include "GE/Project/Project.h"
#include "GE/Scene/Scene.h"

namespace GE
{

	class AssetPack
	{
	public:
		template<typename T>
		Ref<T> GetAsset(UUID handle);

		uint64_t GetBuildVersion();
		bool HandleExists(UUID handle);

		bool AddAsset(Ref<Asset> asset);
		bool RemoveAsset(Ref<Asset> asset);

		bool Serialize(const std::filesystem::path& filePath);
		bool Deserialize(const std::filesystem::path& filePath);

	private:
		std::filesystem::path m_Path;
		AssetPackFile m_File;

		std::unordered_set<UUID> m_HandleIndex;

		friend class RuntimeAssetManager;
	};
}

