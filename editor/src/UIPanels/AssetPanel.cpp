#include "AssetPanel.h"

#include "../AssetManager/EditorAssetManager.h"

#include "GE/Asset/AssetImporter.h"

#include "GE/Core/Core.h"

#include "GE/Project/Project.h"
#include "GE/Scene/Scene.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>


namespace GE
{
	AssetPanel::AssetPanel()
		: m_RootPath(Project::GetAssetPath()), m_CurrentPath(m_RootPath)
	{
		m_TreeNodes.clear();
		m_TreeNodes.push_back(TreeNode(".", 0));
		RefreshAssets();
	}

	void AssetPanel::OnImGuiRender()
	{
		ImGui::Begin("Asset Panel");
		ImGui::Checkbox("View All", &m_ViewFileSystem);

		if (m_CurrentPath != m_RootPath)
		{
			ImGui::SameLine();
			if (ImGui::Button("<- Back"))
			{
				m_CurrentPath = m_CurrentPath.parent_path();
			}
		}

		if (m_ViewFileSystem)
		{
			for (auto& dir : std::filesystem::directory_iterator(m_CurrentPath))
			{
				const auto& path = dir.path();
				auto relativePath = std::filesystem::relative(dir.path(), m_RootPath);
				std::string pathString = relativePath.filename().string();


				bool isDirectory = std::filesystem::is_directory(path);

				if (isDirectory)
				{
					if (ImGui::Button(pathString.c_str()))
					{
						m_CurrentPath /= path.filename();
					}
				}
				else
				{
					if (ImGui::Button(pathString.c_str()))
					{
						
					}
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Import as Asset"))
					{
						Ref<Asset> asset = AssetImporter::ImportAsset(AssetMetadata(relativePath));
						if(asset)
							RefreshAssets();
					}
					ImGui::EndPopup();
				}

			}
		}
		else
		{
			// Get root node(directory)
			TreeNode* node = &m_TreeNodes[0];

			auto currentDir = std::filesystem::relative(m_CurrentPath);
			// Get current node(directory)
			for (const auto& filePath : currentDir)
			{
				if (node->Path == currentDir)
					break;

				if (node->Children.find(filePath) != node->Children.end())
				{
					node = &m_TreeNodes[node->Children[filePath]];
					continue;
				}
			}

			// Display current node(directory)
			for (const auto& [path, index] : node->Children)
			{
				std::string pathString = path.generic_string();
				bool isDirectory = std::filesystem::is_directory(Project::GetPathToAsset(path));

				if (ImGui::Button(pathString.c_str()))
				{
					if(isDirectory)
						m_CurrentPath /= path.filename();
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Delete Asset"))
					{
						Project::GetAssetManager<EditorAssetManager>()->RemoveAsset(m_TreeNodes[index].Handle);
						RefreshAssets();
					}
					ImGui::EndPopup();
				}

				if (ImGui::BeginDragDropSource())
				{
					const UUID handle = m_TreeNodes[index].Handle;
					ImGui::SetDragDropPayload("ASSET_PANEL_ITEM", &handle, sizeof(UUID));

					ImGui::EndDragDropSource();
				}
			}
		}

		ImGui::End();
	}

	void AssetPanel::RefreshAssets()
	{
		const auto& assetRegistry = Project::GetAssetManager<EditorAssetManager>()->GetAssetRegistry();

		for (const auto& [handle, metadata] : assetRegistry)
		{
			uint32_t currentNodeIndex = 0;
			for (const auto& filepath : metadata.FilePath)
			{
				auto it = m_TreeNodes[currentNodeIndex].Children.find(filepath.generic_string());
				if (it != m_TreeNodes[currentNodeIndex].Children.end())
				{
					// Move to next index
					currentNodeIndex = it->second;
				}
				else
				{
					TreeNode newNode(filepath, handle);
					newNode.Parent = currentNodeIndex;
					m_TreeNodes.push_back(newNode);
					m_TreeNodes[currentNodeIndex].Children[filepath] = m_TreeNodes.size() - 1;

					currentNodeIndex = m_TreeNodes.size() - 1;
				}
			}
		}
	}
}