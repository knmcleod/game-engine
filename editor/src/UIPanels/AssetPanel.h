#pragma once

#include "GE/Core/UUID/UUID.h"

#include <filesystem>
#include <map>

namespace GE
{
	class AssetPanel
	{
	public:
		AssetPanel();

		void OnImGuiRender();
	private:
		void RefreshAssets();
	private:
		struct TreeNode
		{
			std::filesystem::path Path;
			UUID Handle = 0;

			uint32_t Parent = (uint32_t)-1;
			// childs filePath & index relative to parent
			std::map<std::filesystem::path, uint32_t> Children;

			TreeNode(const std::filesystem::path& filePath, UUID handle) : Path(filePath), Handle(handle)
			{

			}
		};

		std::vector<TreeNode> m_TreeNodes;

		std::filesystem::path m_RootPath;
		std::filesystem::path m_CurrentPath;

		bool m_ViewFileSystem = true;
	};
}