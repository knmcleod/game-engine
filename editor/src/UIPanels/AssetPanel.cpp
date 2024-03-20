#include "AssetPanel.h"

#include "GE/Core/Core.h"
#include "GE/Project/Project.h"
#include "GE/Scene/Scene.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace GE
{
	AssetPanel::AssetPanel()
		: m_BasePath(Project::GetAssetPath()), m_CurrentPath(m_BasePath)
	{

	}

	void AssetPanel::OnImGuiRender()
	{
		ImGui::Begin("Asset Panel");

		if (m_CurrentPath != m_BasePath)
		{
			if (ImGui::Button("<- Back"))
			{
				m_CurrentPath = m_CurrentPath.parent_path();
			}
		}

		for (auto& dir : std::filesystem::directory_iterator(m_CurrentPath))
		{
			auto path = dir.path();
			auto relativePath = std::filesystem::relative(dir.path(), m_BasePath);
			std::string filenameString = relativePath.filename().string();

			if (dir.is_directory())
			{
				if (ImGui::Button(filenameString.c_str()))
				{
					m_CurrentPath /= dir.path().filename();
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