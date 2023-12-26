#pragma once

#include "GE/Core/Core.h"
#include "GE/Scene/Scene.h"

#include <filesystem>

namespace GE
{
	class AssetPanel
	{
	public:
		AssetPanel();

		void OnImGuiRender();
	private:
		std::filesystem::path m_CurrentDir;
	};
}