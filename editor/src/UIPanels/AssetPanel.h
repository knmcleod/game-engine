#pragma once

#include <filesystem>

namespace GE
{
	class AssetPanel
	{
	public:
		AssetPanel();

		void OnImGuiRender();
	private:
		std::filesystem::path m_BasePath;
		std::filesystem::path m_CurrentPath;
	};
}