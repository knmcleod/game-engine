#include "AssetPanel.h"

#include "../AssetManager/EditorAssetManager.h"

#include "GE/Audio/AudioManager.h"
#include "GE/Asset/Assets/Scene/Scene.h"
#include "GE/Asset/Assets/Textures/Texture.h"
#include "GE/Asset/Assets/Font/Font.h"

#include "GE/Core/Core.h"

#include "GE/Core/Application/Application.h"
#include "GE/Project/Project.h"

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

		AudioManager::GenerateSource(m_AudioSourceID);
	}

	AssetPanel::~AssetPanel()
	{
		AudioManager::RemoveSource(m_AudioSourceID);
	}

	void AssetPanel::OnImGuiRender()
	{
		ImGui::Begin("Asset Panel");
		ImGui::Checkbox("View All", &m_ViewFileSystem);
		ImGui::SameLine();
		if (ImGui::Button("Refresh"))
			RefreshAssets();

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

				if (ImGui::Button(pathString.c_str()))
				{
					if (isDirectory)
					{
						m_CurrentPath /= path.filename();
					}
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Import as Asset"))
					{
						AssetMetadata metadata(UUID(), relativePath);
						if (metadata.Type == Asset::Type::Script)
						{
							// Expected input = scripts/src/namespace/className.cs
							std::filesystem::path className = metadata.FilePath.filename();
							std::filesystem::path nameSpace = className.parent_path();

							// Expected output = namespace/className.cs
							metadata.FilePath = std::filesystem::path(nameSpace / className);
						}
						if (Project::GetAssetManager<EditorAssetManager>()->AddAsset(metadata))
							RefreshAssets();

					}
					ImGui::EndPopup();
				}

			}
		}
		else // Assets only
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
				std::filesystem::path fullPath = m_CurrentPath / path;
				bool isDirectory = std::filesystem::is_directory(fullPath);

				if (ImGui::Button(pathString.c_str()))
				{
					if (isDirectory)
						m_CurrentPath /= path.filename();
					else
						m_CurrentAsset = Project::GetAssetManager()->GetAsset(m_TreeNodes.at(index).Handle);
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Delete Asset"))
					{
						if (Project::GetAssetManager<EditorAssetManager>()->RemoveAsset(m_TreeNodes.at(index).Handle))
							RefreshAssets();
						else
							GE_CORE_ERROR("Failed to Remove Asset.");
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

		ImGui::Begin("Asset Viewer");

		if (m_CurrentAsset)
		{
			switch (m_CurrentAsset->GetType())
			{
			case Asset::Type::Texture2D:
			{
				Ref<Texture2D> texture = Project::GetAssetAs<Texture2D>(m_CurrentAsset);
				ImGui::Image((ImTextureID)(uint64_t)texture->GetID(), ImVec2((float)texture->GetWidth(), (float)texture->GetHeight()));
			}
			break;
			case Asset::Type::Font:
			{
				Ref<Font> font = Project::GetAssetAs<Font>(m_CurrentAsset);
				if (font)
					ImGui::Image((ImTextureID)(uint64_t)font->GetAtlasTexture()->GetID(), ImVec2((float)font->GetWidth(), (float)font->GetHeight()));
			}
			break;
			case Asset::Type::Audio:
			{
				Ref<Audio> audioAsset = Project::GetAssetAs<Audio>(m_CurrentAsset);

				ImGui::Checkbox("Loop", &m_LoopAudio);
				ImGui::DragFloat("Pitch", &m_AudioPitch, .25, 0.0);
				ImGui::DragFloat("Gain", &m_AudioGain, .25, 0.0);

				glm::vec3 position = glm::vec3(0.0);
				glm::vec3 velocity = glm::vec3(0.0);

				if (ImGui::Button("Play"))
				{
					Application::SubmitToMainAppThread([sourceID = m_AudioSourceID, loop = m_LoopAudio, pitch = m_AudioPitch, gain = m_AudioGain, p = position, v = velocity, audio = audioAsset]()
						{
							AudioManager::Play(sourceID, loop, pitch, gain, p, v, audio);
						});
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop") && AudioManager::IsSourcePlaying(m_AudioSourceID))
					AudioManager::Stop(m_AudioSourceID);
			}
			break;
			}
		}

		ImGui::End();
	}

	void AssetPanel::RefreshAssets()
	{
		const auto& assetRegistry = Project::GetAssetManager<EditorAssetManager>()->GetAssetRegistry()->GetRegistry();

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
					m_TreeNodes[currentNodeIndex].Children[filepath] = (unsigned int)m_TreeNodes.size() - 1;

					currentNodeIndex = (uint32_t)m_TreeNodes.size() - 1;
				}
			}
		}
	}
}