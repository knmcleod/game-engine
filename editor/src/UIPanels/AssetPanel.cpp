#include "AssetPanel.h"

#include "GE/Core/Core.h"
#include "GE/Scene/Scene.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace GE
{
	extern const std::filesystem::path g_AssetsPath = "assets";

	AssetPanel::AssetPanel() : m_CurrentDir(g_AssetsPath)
	{

	}

	void AssetPanel::OnImGuiRender()
	{
		ImGui::Begin("Asset Panel");

		if (m_CurrentDir != std::filesystem::path(g_AssetsPath))
		{
			if (ImGui::Button("<- Back"))
			{
				m_CurrentDir = m_CurrentDir.parent_path();
			}
		}

		for (auto& dir : std::filesystem::directory_iterator(m_CurrentDir))
		{
			auto path = dir.path();
			auto relativePath = std::filesystem::relative(dir.path(), g_AssetsPath);
			std::string filenameString = relativePath.filename().string();

			if (dir.is_directory())
			{
				if (ImGui::Button(filenameString.c_str()))
				{
					m_CurrentDir /= dir.path().filename();
				}
			}
			else
			{
				if (ImGui::Button(filenameString.c_str()))
				{

				}
			}

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("ASSET_PANEL_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));

				ImGui::EndDragDropSource();
			}
		}

		ImGui::End();
	}
}