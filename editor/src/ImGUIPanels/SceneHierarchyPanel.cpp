#include "SceneHierarchyPanel.h"

#include "../Application/Layer/EditorLayerStack.h"
#include "../AssetManager/EditorAssetManager.h"
#include "../Project/EditorProject.h"

#include "GE/Asset/Assets/Font/Font.h"
#include "GE/Audio/AudioManager.h"
#include "GE/Core/Application/Application.h"
#include "GE/Core/FileSystem/FileSystem.h"

#include "GE/Project/Project.h"
#include "GE/Scripting/Scripting.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>

namespace GE
{
#pragma region ImGUIDraw

	template<typename T, typename UIFunction>
	static void DrawComponent(Ref<Scene> scene, const std::string& name, Entity entity, UIFunction function)
	{
		if (scene->HasComponent<T>(entity))
		{
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
				| ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_SpanAvailWidth;

			auto& component = scene->GetComponent<T>(entity);
			bool removeComponent = false;

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::SameLine();
			if (ImGui::Button("+"))
				ImGui::OpenPopup("ComponentSettings");

			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove"))
					removeComponent = true;
				ImGui::EndPopup();
			}

			if (open)
			{
				function(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				scene->RemoveComponent<T>(entity);
		}
	}

	template<typename T>
	static void DrawAddComponent(Ref<Scene> scene, const std::string& name, Entity entity)
	{
		if (!scene->HasComponent<T>(entity) && ImGui::MenuItem(name.c_str()))
		{
			scene->AddComponent<T>(entity);

			ImGui::CloseCurrentPopup();
		}
	}

	template<typename UIFunc>
	void SceneHierarchyPanel::DrawPopup(const std::string& name, Entity entity, UIFunc func)
	{
		if (ImGui::Button(name.c_str()))
			ImGui::OpenPopup(name.c_str());

		if (ImGui::BeginPopup(name.c_str()))
		{
			func(entity);
			ImGui::EndPopup();
		}
	}

	void SceneHierarchyPanel::DrawInputText(const std::string& label, std::string& text)
	{
		if (label.empty())
			return;
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), text.c_str());
		if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer)))
			text.assign(buffer);
	}

#pragma endregion

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::SetSelected(UUID selectedEntity /*= 0*/)
	{
		m_SelectedEntityID = selectedEntity;
	}

	void SceneHierarchyPanel::OnImGuiRender(Ref<Scene> scene)
	{
		if (scene && ImGui::Begin("Scene Hierarchy"))
		{
			std::vector<Entity> entities = scene->GetAllEntitiesWith<RelationshipComponent>();
			for (Entity entity : entities)
			{
				auto& idC = scene->GetComponent<IDComponent>(entity);
				auto& rc = scene->GetComponent<RelationshipComponent>(entity);
				// Only draw parent Entities in main Hierarchy panel
				// ChildEntities are handled inside the parent
				if (rc.GetParent() == idC.ID)
					DrawEntity(scene, entity);

			}
			entities.clear();
			entities = std::vector<Entity>();

			ImGui::Dummy(ImGui::GetContentRegionMax());
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_PANEL_ITEM"))
				{
					const UUID entityID = *(UUID*)payload->Data;
					Entity entity = scene->GetEntityByUUID(entityID);

					auto& entityRC = scene->GetOrAddComponent<RelationshipComponent>(entity);
					if (entityRC.GetParent() != entityID) // Entity is a child
					{
						scene->SetEntityParent(entity, entityID);
					}

				}
				ImGui::EndDragDropTarget();
			}


			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				m_SelectedEntityID = 0;
			}

			//	Right-Click Blank space
			if (ImGui::BeginPopupContextWindow(0))
			{
				if (ImGui::MenuItem("Create New Entity"))
				{
					scene->CreateEntity("New Entity", 0);
				}

				ImGui::EndPopup();
			}

			ImGui::End();
		}
		
		if (scene && ImGui::Begin("Entity Components"))
		{
			if (Entity entity = scene->GetEntityByUUID(m_SelectedEntityID))
				DrawComponents(scene, entity);

			ImGui::End();
		}
	}

	static std::string GetStringFromGUIState(const GUIState& state)
	{
		switch (state)
		{
		case GUIState::Disabled:
			return "Disabled";
			break;
		case GUIState::Enabled:
			return "Enabled";
			break;
		case GUIState::Hovered:
			return "Hovered";
			break;
		case GUIState::Focused:
			return "Focused";
			break;
		case GUIState::Active:
			return "Active";
			break;
		case GUIState::Selected:
			return "Selected";
			break;
		default:
			break;
		}
	}
	void SceneHierarchyPanel::DrawEntity(Ref<Scene> scene, Entity entity)
	{
		if (!scene || !entity)
			return;

		auto& name = scene->GetComponent<NameComponent>(entity).Name;
		auto& id = scene->GetComponent<IDComponent>(entity).ID;
		auto& rc = scene->GetOrAddComponent<RelationshipComponent>(entity);

		ImGuiTreeNodeFlags flags = ((m_SelectedEntityID == id) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
			
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectedEntityID = id;
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_PANEL_ITEM"))
			{
				const UUID childEntityID = *(UUID*)payload->Data;
				if (childEntityID != id) // cannot parent self here, only by dragging into hierarchy panel
				{
					if (Entity childEntity = scene->GetEntityByUUID(childEntityID))
					{
						rc.AddChild(childEntityID);

						scene->SetEntityParent(childEntity, id);
					}
				}
				else
					GE_WARN("SceneHierarchyPanel::DrawEntity(Entity) - Tried to add self to children.");
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginDragDropSource())
		{
			const UUID entityID = scene->GetComponent<IDComponent>(entity).ID;
			ImGui::SetDragDropPayload("SCENE_PANEL_ITEM", &entityID, sizeof(UUID));

			ImGui::EndDragDropSource();
		}

		bool entityDeleted = false;
		//	Right-Click Blank space

		if (ImGui::BeginPopupContextWindow(0))
		{
			if (ImGui::MenuItem("DeleteEntity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		// Show child entities if present
		if (opened)
		{
			for (const UUID& child : rc.GetChildren())
			{
				Entity childEntity = scene->GetEntityByUUID(child);
				DrawEntity(scene, childEntity);
			}

			// Pop Parent Entity
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			if (m_SelectedEntityID == id)
				m_SelectedEntityID = 0;
			if (scene)
				scene->DestroyEntity(entity);
		}
	}

	void SceneHierarchyPanel::DrawComponents(Ref<Scene> scene, Entity entity)
	{
		if (scene->HasComponent<NameComponent>(entity))
		{
			auto& name = scene->GetComponent<NameComponent>(entity).Name;

			DrawInputText("\n", name);
		}

		ImGui::SameLine();
		DrawPopup("+", entity, [scene](auto& entity)
			{
				DrawAddComponent<TransformComponent>(scene, "Transform", entity);

				DrawPopup("Audio", entity, [scene](auto& entity)
					{
						DrawAddComponent<AudioSourceComponent>(scene, "Audio Source", entity);
						DrawAddComponent<AudioListenerComponent>(scene, "Audio Listener", entity);
					});

				DrawPopup("Rendering", entity, [scene](auto& entity)
					{
						DrawAddComponent<CameraComponent>(scene, "Camera", entity);

						DrawAddComponent<SpriteRendererComponent>(scene, "Sprite Renderer", entity);
						DrawAddComponent<CircleRendererComponent>(scene, "Circle Renderer", entity);
						DrawAddComponent<TextRendererComponent>(scene, "Text Renderer", entity);

						DrawPopup("UI", entity, [scene](auto& entity)
							{
								DrawAddComponent<GUICanvasComponent>(scene, "Canvas", entity);
								DrawAddComponent<GUILayoutComponent>(scene, "Layout", entity);
								// TODO : GUIMaskComponent
								DrawAddComponent<GUIImageComponent>(scene, "Image", entity);
								DrawAddComponent<GUIButtonComponent>(scene, "Button", entity);
								DrawAddComponent<GUIInputFieldComponent>(scene, "Input Field", entity);
								DrawAddComponent<GUISliderComponent>(scene, "Slider", entity);
								DrawAddComponent<GUICheckboxComponent>(scene, "Checkbox", entity);
								// TODO : GUIScrollRectComponent & GUIScrollbarComponent
							});
					});

				DrawPopup("Scripting", entity, [scene](auto& entity)
					{
						DrawAddComponent<NativeScriptComponent>(scene, "Native Script", entity);
						DrawAddComponent<ScriptComponent>(scene, "Script", entity);
					});

				DrawPopup("Physics", entity, [scene](auto& entity)
					{
						DrawAddComponent<Rigidbody2DComponent>(scene, "Rigidbody 2D", entity);
						DrawAddComponent<BoxCollider2DComponent>(scene, "Box Collider 2D", entity);
						DrawAddComponent<CircleCollider2DComponent>(scene, "Circle Collider 2D", entity);
					});
			});
		ImGui::SameLine();
		if (scene->HasComponent<ActiveComponent>(entity))
		{
			auto& ac = scene->GetComponent<ActiveComponent>(entity);
			ImGui::Checkbox("A", &ac.Active);
			ImGui::SameLine();
			ImGui::Checkbox("H", &ac.Hidden);

			// TODO : use image buttons, similar to EditorLayer::ImGUI_SceneToolbar(Ref<Scene>)
		}

		if (scene->HasComponent<TagComponent>(entity))
		{
			auto& tc = scene->GetComponent<TagComponent>(entity);
			std::string tagStr = Project::GetStrByTag(tc.TagID);
			if (tagStr.empty())
				tagStr = std::string("None");

			ImGui::Text("Tag:");
			ImGui::SameLine();
			if (ImGui::BeginMenu(tagStr.c_str()))
			{
				for (const auto& [tag, string] : Project::GetTags())
				{
					bool isSelected = (tc.TagID == tag);
					if (!string.empty() && ImGui::MenuItem(string.c_str(), nullptr, isSelected))
					{
						GE_TRACE("Selected Tag : {0}, ID : {1}", string.c_str(), tag);
						if (isSelected) // Clear if already selected
							tc.TagID = 0;
						else // Set if not
							tc.TagID = tag;
					}
				}

				ImGui::Separator();
				DrawInputText("\n", m_NewTagStr);
				ImGui::SameLine();
				if (ImGui::Button("+"))
				{
					if (!m_NewTagStr.empty())
					{
						if (Project::AddTag(m_NewTagStr, (uint32_t)Project::GetTags().size()))
							m_NewTagStr.clear();
					}
				}

				ImGui::EndMenu();
			}

		}

		if (scene->HasComponent<RenderComponent>(entity))
		{
			Ref<EditorLayerStack> editorLayerStack = Application::GetLayerStack<EditorLayerStack>();
			auto& rc = scene->GetComponent<RenderComponent>(entity);
			std::string layerStr = std::string();
			if (rc.LayerIDs.empty())
				layerStr = std::string("None");
			else if (rc.LayerIDs.size() == 1)
				layerStr = editorLayerStack->GetLayerName(rc.LayerIDs[0]);
			else
				layerStr = std::string("Multi");
			ImGui::Text("Layer:");
			ImGui::SameLine();
			if (ImGui::BeginMenu(layerStr.c_str()))
			{
				for (const auto& [id, layer] : Application::GetLayers())
				{
					uint64_t layerID = layer->GetID();
					bool isSelected = rc.IDHandled(layerID);
					if (ImGui::MenuItem(editorLayerStack->GetLayerName(layerID).c_str(), nullptr, isSelected))
					{
						if (!isSelected)
							rc.AddID(layerID);
						else
							rc.RemoveID(layerID);
					}
				}

				ImGui::Separator();
				DrawInputText("\n", m_NewLayerStr);
				ImGui::SameLine();
				if (ImGui::Button("+"))
				{
					if (!m_NewLayerStr.empty())
					{
						Application::SubmitToMainAppThread([newLayerStr = m_NewLayerStr, els = editorLayerStack]()
							{
								Ref<Layer> layer = CreateRef<Layer>();
								if (els->InsertLayer(layer, newLayerStr))
									GE_TRACE("Successfully added new Layer to EditorLayerStack.");
							});
						m_NewLayerStr.clear();
					}
				}
				ImGui::EndMenu();
			}
		}

		ImGui::Separator();
		DrawComponent<TransformComponent>(scene, "Transform", entity,
			[PivotStrs = EditorAssetManager::GetPivotStrs()](auto& component)
			{
				ImGui::DragFloat3("Position", glm::value_ptr(component.Translation));
				ImGui::DragFloat3("Rotation", glm::value_ptr(component.Rotation));
				ImGui::DragFloat3("Scale", glm::value_ptr(component.Scale));

				const std::string pivotStr = EditorAssetManager::PivotToString(component.GetPivot());
				ImGui::Text("Pivot:");
				ImGui::SameLine();
				if (ImGui::BeginMenu(pivotStr.c_str()))
				{
					for (const auto& [pivot, str] : PivotStrs)
					{
						bool isSelected = pivotStr == str;
						if (ImGui::MenuItem(str.c_str(), nullptr, isSelected))
						{
							component.SetPivot(pivot);
						}
					}
					ImGui::EndMenu();
				}
				// TODO : Custom Pivot(?)
				//ImGui::DragFloat3("Pivot", glm::value_ptr(component.PivotOffset));
			});

#pragma region Audio

		DrawComponent<AudioSourceComponent>(scene, "Audio Source", entity,
			[](auto& component)
			{
				ImGui::Checkbox("Loop", &component.Loop);
				ImGui::DragFloat("Gain", &component.Gain);
				ImGui::DragFloat("Pitch", &component.Pitch);

				ImGui::Text("Audio Asset");
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
					{
						const UUID handle = *(UUID*)payload->Data;

						Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
						Asset::Type type = asset->GetType();
						if (type == Asset::Type::Audio)
						{
							component.AssetHandle = asset->GetHandle();
						}
						else
						{
							GE_WARN("Asset Type is not AudioClip.");
						}
					}
					ImGui::EndDragDropTarget();
				}
			});

		DrawComponent<AudioListenerComponent>(scene, "Audio Listener", entity,
			[](auto& component)
			{
				ImGui::Text("Device", AudioManager::GetDeviceName());
			});
#pragma endregion

#pragma region Rendering

		DrawComponent<CameraComponent>(scene, "Camera", entity,
			[](CameraComponent& cc)
			{
				const SceneCamera& camera = cc.ActiveCamera;

				ImGui::Checkbox("Primary", &cc.Primary);
				ImGui::Checkbox("Fixed Aspect Ratio", &cc.FixedAspectRatio);
				float aspectRatio = camera.GetAspectRatio();
				ImGui::DragFloat("Aspect Ratio", &aspectRatio);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int type = 0; type < 2; type++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[type];
						if (ImGui::Selectable(projectionTypeStrings[type], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[type];
							cc.SetProjectionType((Camera::ProjectionType)type);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				float fov = camera.GetFOV();
				if (ImGui::DragFloat("FOV", &fov))
					cc.SetFOV(fov);

				float nearClip = camera.GetNearClip();
				if (ImGui::DragFloat("Near Clip", &nearClip))
					cc.SetNearClip(nearClip);

				float farClip = camera.GetFarClip();
				if (ImGui::DragFloat("Far Clip", &farClip))
					cc.SetFarClip(farClip);

			});

		DrawComponent<SpriteRendererComponent>(scene, "Sprite Renderer", entity,
			[](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
				ImGui::DragFloat("Tiling Factor", &component.TilingFactor);
				{
					if (ImGui::Button("Texture"))
					{
						std::string filePath = FileSystem::LoadFromFileDialog("PNG(*.png)\0*.png\0");
						if (!filePath.empty())
						{
							Ref<Asset> texture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
							if (texture)
							{
								component.AssetHandle = texture->GetHandle();
							}
						}
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset)
							{
								if (asset->GetType() == Asset::Type::Texture2D)
								{
									component.AssetHandle = asset->GetHandle();
								}
								else
								{
									GE_WARN("Asset Type is not Texture2D.");
								}
							}
							else
							{
								GE_ERROR("Cannot assign Asset. Asset not found in AssetManager.");
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
				
			});

		DrawComponent<CircleRendererComponent>(scene, "Circle Renderer", entity,
			[](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
				ImGui::DragFloat("Thickness", &component.Thickness, 0.25f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.Fade, 0.25f, 0.0f, 1.0f);

			});

		DrawComponent<TextRendererComponent>(scene, "Text Renderer", entity,
			[](TextRendererComponent& trc)
			{
				ImGui::ColorEdit4("Text Color", glm::value_ptr(trc.TextColor));
				ImGui::ColorEdit4("Background Color", glm::value_ptr(trc.BGColor));

				ImGui::DragFloat("Width Spacing", &trc.KerningOffset);
				ImGui::DragFloat("Height Spacing", &trc.LineHeightOffset);
				ImGui::DragFloat("TextScalar", &trc.TextScalar);
				ImGui::DragFloat2("TextOffset", glm::value_ptr(trc.TextOffset));

				std::string& text = trc.Text;

				char* buffer = new char[sizeof(std::string)];
				memset(buffer, 0, sizeof(std::string));
				if (!text.empty())
					strcpy_s(buffer, sizeof(std::string), text.c_str());
				if (ImGui::InputTextMultiline("Text", buffer, sizeof(std::string)))
					text = std::string(buffer);
				delete[](buffer);
				{
					Ref<Asset> font = nullptr;
					if (ImGui::Button("Font"))
					{
						std::string filePath = FileSystem::LoadFromFileDialog("TTF(*.ttf)\0*.ttf\0");
						if (!filePath.empty())
						{
							font = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
							if (font)
							{
								trc.AssetHandle = font->GetHandle();

							}
						}
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Font)
							{
								trc.AssetHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Font.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (font)
					{
						Ref<Texture2D> fontAtlas = Project::GetAsset<Font>(font->GetHandle())->GetAtlasTexture();
						if (fontAtlas != nullptr)
							ImGui::Image((ImTextureID)(uint64_t)fontAtlas->GetID(), { 512, 512 }, { 0, 1 }, { 1, 0 });
					}
				}

			});

#pragma region UI
		DrawComponent<GUIComponent>(scene, "GUI Component", entity,
			[](GUIComponent& guiC)
			{
				std::string lastStateStr = "Last State : " + GetStringFromGUIState(guiC.LastState);
				ImGui::Text(lastStateStr.c_str());

				std::string currentStateStr = "Current State : " + GetStringFromGUIState(guiC.CurrentState);
				ImGui::Text(currentStateStr.c_str());

				ImGui::Checkbox("IsNavigatable", &guiC.IsNavigatable);
			});

		DrawComponent<GUICanvasComponent>(scene, "GUI Canvas", entity,
			[CanvasModeStrs = EditorAssetManager::GetCanvasModeStrs()](GUICanvasComponent& guiCC)
			{
				ImGui::Checkbox("Control Mouse", &guiCC.ControlMouse);
				ImGui::Checkbox("Show Mouse", &guiCC.ShowMouse);
				const std::string modeStr = EditorAssetManager::CanvasModeToStr(guiCC.Mode);
				ImGui::Text("Mode:");
				ImGui::SameLine();
				if (ImGui::BeginMenu(modeStr.c_str()))
				{
					for (const auto& [mode, str] : CanvasModeStrs)
					{
						bool isSelected = modeStr == str;
						if (ImGui::MenuItem(str.c_str(), nullptr, isSelected))
						{
							guiCC.Mode = mode;
						}
					}
					ImGui::EndMenu();
				}
			});

		DrawComponent<GUILayoutComponent>(scene, "GUI Layout", entity,
			[LayoutModeStrs = EditorAssetManager::GetLayoutModeStrs()](GUILayoutComponent& guiLOC)
			{
				const std::string modeStr = EditorAssetManager::LayoutModeToStr(guiLOC.Mode);
				ImGui::Text("Mode:");
				ImGui::SameLine();
				if (ImGui::BeginMenu(modeStr.c_str()))
				{
					for (const auto& [mode, str] : LayoutModeStrs)
					{
						bool isSelected = modeStr == str;
						if (ImGui::MenuItem(str.c_str(), nullptr, isSelected))
						{
							guiLOC.Mode = mode;
						}
					}
					ImGui::EndMenu();
				}
				ImGui::DragFloat2("StartingOffset", glm::value_ptr(guiLOC.StartingOffset));
				ImGui::DragFloat2("ChildSize", glm::value_ptr(guiLOC.ChildSize));
				ImGui::DragFloat2("ChildPadding", glm::value_ptr(guiLOC.ChildPadding));
			});
		
		// TODO : GUIMaskComponent

		DrawComponent<GUIImageComponent>(scene, "GUI Image", entity,
			[](GUIImageComponent& guiIC)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(guiIC.Color));
				ImGui::Text("Texture");
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
					{
						const UUID handle = *(UUID*)payload->Data;

						Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
						if (asset->GetType() == Asset::Type::Texture2D)
						{
							guiIC.TextureHandle = asset->GetHandle();
						}
						else
						{
							GE_WARN("Asset Type is not Texture2D.");
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::DragFloat("Tiling Factor", &guiIC.TilingFactor);

			});

		DrawComponent<GUIButtonComponent>(scene, "GUI Button", entity,
			[](GUIButtonComponent& guiBC)
			{
				// Text
				{
					ImGui::ColorEdit4("Text Color", glm::value_ptr(guiBC.TextColor));
					ImGui::ColorEdit4("Background Color", glm::value_ptr(guiBC.BGColor));

					ImGui::DragFloat("Kerning", &guiBC.KerningOffset);
					ImGui::DragFloat("Line Height Spacing", &guiBC.LineHeightOffset);

					std::string& text = guiBC.Text;

					char* buffer = new char[256];
					memset(buffer, 0, sizeof(buffer));
					if (!text.empty())
						strcpy_s(buffer, sizeof(buffer), text.c_str());
					if (ImGui::InputTextMultiline("Text", buffer, sizeof(buffer)))
						text = std::string(buffer);
					delete[](buffer);

					ImGui::Text("Font Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Font)
							{
								guiBC.FontAssetHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Font.");
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
				
				// Textures + Colors
				{
					ImGui::ColorEdit4("Background Color", glm::value_ptr(guiBC.BackgroundColor));
					ImGui::Text("Background Texture");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiBC.BackgroundTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Disabled Color", glm::value_ptr(guiBC.DisabledColor));
					ImGui::Text("Disabled Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiBC.DisabledTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Enabled Color", glm::value_ptr(guiBC.EnabledColor));
					ImGui::Text("Enabled Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiBC.EnabledTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Hovered Color", glm::value_ptr(guiBC.HoveredColor));
					ImGui::Text("Hovered Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiBC.HoveredTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Selected Color", glm::value_ptr(guiBC.SelectedColor));
					ImGui::Text("Selected Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiBC.SelectedTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Foreground Color", glm::value_ptr(guiBC.ForegroundColor));
					ImGui::Text("Foreground Texture");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiBC.ForegroundTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}
				}

			});

		DrawComponent<GUIInputFieldComponent>(scene, "GUI Input Field", entity,
			[](GUIInputFieldComponent& guiIFC)
			{
				// Font
				{
					ImGui::ColorEdit4("Text Color", glm::value_ptr(guiIFC.TextColor));
					ImGui::ColorEdit4("Background Color", glm::value_ptr(guiIFC.BGColor));

					ImGui::DragFloat("Width Spacing", &guiIFC.KerningOffset);
					ImGui::DragFloat("Height Spacing", &guiIFC.LineHeightOffset);
					ImGui::DragFloat("Text Size Scalar", &guiIFC.TextScalar);
					ImGui::DragFloat2("Text Starting Offset", glm::value_ptr(guiIFC.TextStartingOffset));
					ImGui::DragFloat2("Text Padding", glm::value_ptr(guiIFC.Padding));

					std::string& text = guiIFC.Text;

					char* buffer = new char[sizeof(std::string)];
					memset(buffer, 0, sizeof(std::string));
					if (!text.empty())
						strcpy_s(buffer, sizeof(std::string), text.c_str());
					if (ImGui::InputTextMultiline("Input", buffer, sizeof(std::string)))
						text = std::string(buffer);
					delete[](buffer);
					{
						Ref<Asset> font = nullptr;
						if (ImGui::Button("Font"))
						{
							std::string filePath = FileSystem::LoadFromFileDialog("TTF(*.ttf)\0*.ttf\0");
							if (!filePath.empty())
							{
								font = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
								if (font)
								{
									guiIFC.FontAssetHandle = font->GetHandle();

								}
							}
						}
						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
							{
								const UUID handle = *(UUID*)payload->Data;

								Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
								if (asset->GetType() == Asset::Type::Font)
								{
									guiIFC.FontAssetHandle = asset->GetHandle();
								}
								else
								{
									GE_WARN("Asset Type is not Font.");
								}
							}
							ImGui::EndDragDropTarget();
						}

						if (font)
						{
							Ref<Texture2D> fontAtlas = Project::GetAsset<Font>(font->GetHandle())->GetAtlasTexture();
							if (fontAtlas != nullptr)
								ImGui::Image((ImTextureID)(uint64_t)fontAtlas->GetID(), { 512, 512 }, { 0, 1 }, { 1, 0 });
						}
					}
				}
				
				// Texture
				{
					ImGui::ColorEdit4("Color", glm::value_ptr(guiIFC.BackgroundColor));
					ImGui::Checkbox("Fill BG", &guiIFC.FillBackground);
					ImGui::DragFloat2("Fill Size", glm::value_ptr(guiIFC.TextSize));
					if(ImGui::Button("Texture"))
					{
						std::string filePath = FileSystem::LoadFromFileDialog("PNG(*.png)\0*.png\0");
						if (!filePath.empty())
						{
							Ref<Asset> texture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
							if (texture)
							{
								guiIFC.BackgroundTextureHandle = texture->GetHandle();
							}
						}
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset)
							{
								if (asset->GetType() == Asset::Type::Texture2D)
								{
									guiIFC.BackgroundTextureHandle = asset->GetHandle();
								}
								else
								{
									GE_WARN("Asset Type is not Texture2D.");
								}
							}
							else
							{
								GE_ERROR("Cannot assign Asset. Asset not found in AssetManager.");
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
			});

		DrawComponent<GUISliderComponent>(scene, "GUI Slider", entity,
			[SliderDirectionStrs = EditorAssetManager::GetSliderDirStrs()](GUISliderComponent& guiSC)
			{
				const std::string directionStr = EditorAssetManager::SliderDirectionToString(guiSC.Direction);
				ImGui::Text("Direction:");
				ImGui::SameLine();
				if (ImGui::BeginMenu(directionStr.c_str()))
				{
					for (const auto& [direction, str] : SliderDirectionStrs)
					{
						bool isSelected = directionStr == str;
						if (ImGui::MenuItem(str.c_str(), nullptr, isSelected))
						{
							guiSC.Direction = direction;
						}
					}
					ImGui::EndMenu();
				}

				ImGui::DragFloat("Fill", &guiSC.Fill, 0.25f, 0.0f, 1.0f);

				// Textures
				{
					ImGui::ColorEdit4("BG Color", glm::value_ptr(guiSC.BackgroundColor));
					if (ImGui::Button("BG Texture"))
					{
						std::string filePath = FileSystem::LoadFromFileDialog("PNG(*.png)\0*.png\0");
						if (!filePath.empty())
						{
							Ref<Asset> texture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
							if (texture)
							{
								guiSC.BackgroundTextureHandle = texture->GetHandle();
							}
						}
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset)
							{
								if (asset->GetType() == Asset::Type::Texture2D)
								{
									guiSC.BackgroundTextureHandle = asset->GetHandle();
								}
								else
								{
									GE_WARN("Asset Type is not Texture2D.");
								}
							}
							else
							{
								GE_ERROR("Cannot assign Asset. Asset not found in AssetManager.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Disabled Color", glm::value_ptr(guiSC.DisabledColor));
					ImGui::Text("Disabled Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiSC.DisabledTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Enabled Color", glm::value_ptr(guiSC.EnabledColor));
					ImGui::Text("Enabled Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiSC.EnabledTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Hovered Color", glm::value_ptr(guiSC.HoveredColor));
					ImGui::Text("Hovered Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiSC.HoveredTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Selected Color", glm::value_ptr(guiSC.SelectedColor));
					ImGui::Text("Selected Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiSC.SelectedTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("FG Color", glm::value_ptr(guiSC.ForegroundColor));
					if (ImGui::Button("FG Texture"))
					{
						std::string filePath = FileSystem::LoadFromFileDialog("PNG(*.png)\0*.png\0");
						if (!filePath.empty())
						{
							Ref<Asset> texture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
							if (texture)
							{
								guiSC.ForegroundTextureHandle = texture->GetHandle();
							}
						}
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset)
							{
								if (asset->GetType() == Asset::Type::Texture2D)
								{
									guiSC.ForegroundTextureHandle = asset->GetHandle();
								}
								else
								{
									GE_WARN("Asset Type is not Texture2D.");
								}
							}
							else
							{
								GE_ERROR("Cannot assign Asset. Asset not found in AssetManager.");
							}
						}
						ImGui::EndDragDropTarget();
					}

				}
			});
			
		DrawComponent<GUICheckboxComponent>(scene, "GUI Checkbox", entity,
				[](GUICheckboxComponent& guiCB)
				{
					ImGui::ColorEdit4("BG Color", glm::value_ptr(guiCB.BackgroundColor));
					if (ImGui::Button("BGTexture"))
				{
					std::string filePath = FileSystem::LoadFromFileDialog("PNG(*.png)\0*.png\0");
					if (!filePath.empty())
					{
						Ref<Asset> texture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
						if (texture)
						{
							guiCB.BackgroundTextureHandle = texture->GetHandle();
						}
					}
				}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset)
							{
								if (asset->GetType() == Asset::Type::Texture2D)
								{
									guiCB.BackgroundTextureHandle = asset->GetHandle();
								}
								else
								{
									GE_WARN("Asset Type is not Texture2D.");
								}
							}
							else
							{
								GE_ERROR("Cannot assign Asset. Asset not found in AssetManager.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Disabled Color", glm::value_ptr(guiCB.DisabledColor));
					ImGui::Text("Disabled Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiCB.DisabledTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Enabled Color", glm::value_ptr(guiCB.EnabledColor));
					ImGui::Text("Enabled Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiCB.EnabledTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Hovered Color", glm::value_ptr(guiCB.HoveredColor));
					ImGui::Text("Hovered Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiCB.HoveredTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("Selected Color", glm::value_ptr(guiCB.SelectedColor));
					ImGui::Text("Selected Texture Asset");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset->GetType() == Asset::Type::Texture2D)
							{
								guiCB.SelectedTextureHandle = asset->GetHandle();
							}
							else
							{
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::ColorEdit4("FG Color", glm::value_ptr(guiCB.ForegroundColor));
					if (ImGui::Button("FG Texture"))
					{
						std::string filePath = FileSystem::LoadFromFileDialog("PNG(*.png)\0*.png\0");
						if (!filePath.empty())
						{
							Ref<Asset> texture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(filePath);
							if (texture)
							{
								guiCB.ForegroundTextureHandle = texture->GetHandle();
							}
						}
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
						{
							const UUID handle = *(UUID*)payload->Data;

							Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
							if (asset)
							{
								if (asset->GetType() == Asset::Type::Texture2D)
								{
									guiCB.ForegroundTextureHandle = asset->GetHandle();
								}
								else
								{
									GE_WARN("Asset Type is not Texture2D.");
								}
							}
							else
							{
								GE_ERROR("Cannot assign Asset. Asset not found in AssetManager.");
							}
						}
						ImGui::EndDragDropTarget();
					}
				});

		// TODO : GUIScrollRectComponent & GUIScrollbarComponent

#pragma endregion

#pragma endregion

#pragma region Scripting

		DrawComponent<NativeScriptComponent>(scene, "Native Script", entity,
			[](auto& component)
			{
			});

		DrawComponent<ScriptComponent>(scene, "Script", entity,
			[entity, scene](ScriptComponent& sc) mutable
			{
				bool exists = Scripting::ScriptExists(sc.AssetHandle);
				if (!exists)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0));

				std::string selectedScriptStr = (exists ? Scripting::GetScript(sc.AssetHandle)->GetFullName() : std::string("None"));
				if (ImGui::BeginMenu(selectedScriptStr.c_str()))
				{
					for (const auto& [fullName, script] : Scripting::GetScripts())
					{
						bool selected = (script->GetHandle() == sc.AssetHandle);
						if (ImGui::MenuItem(fullName.c_str(), nullptr, selected))
							sc.AssetHandle = script->GetHandle();
					}
					ImGui::EndMenu();
				}

				// Script Fields
				if (exists)
				{
					if (Ref<Script> script = Scripting::GetScript(sc.AssetHandle))
					{
						UUID entityUUID = scene->GetComponent<IDComponent>(entity).ID;
						const ScriptFieldMap& classFieldMaps = script->GetFields();
						if (scene->IsStopped())
						{
							ScriptFieldMap& instanceFieldMap = Scripting::GetEntityFields(entityUUID);
							for (const auto& [name, field] : classFieldMaps)
							{
								bool fieldExists = instanceFieldMap.find(name) != instanceFieldMap.end();
								ScriptField& instanceField = instanceFieldMap[name];
								ScriptField::Type type = field.GetType();
								instanceField.SetType(type);
								switch (type)
								{
								case ScriptField::Type::None:
									break;
								case ScriptField::Type::Char:
									break;
								case ScriptField::Type::Int:
								{
									int data = (fieldExists ? instanceField.GetValue<int>() : field.GetValue<int>());
									if (ImGui::DragInt(name.c_str(), &data))
										instanceField.SetValue<int>(data);
								}
									break;
								case ScriptField::Type::UInt:
									break;
								case ScriptField::Type::Float:
								{
									float data = (fieldExists ? instanceField.GetValue<float>() : field.GetValue<float>());
									if (ImGui::DragFloat(name.c_str(), &data))
										instanceField.SetValue<float>(data);
								}
									break;
								case ScriptField::Type::Byte:
									break;
								case ScriptField::Type::Bool:
								{
									bool data = (fieldExists ? instanceField.GetValue<bool>() : field.GetValue<bool>());
									if (ImGui::Checkbox(name.c_str(), &data))
										instanceField.SetValue<bool>(data);
								}
									break;
								case ScriptField::Type::Vector2:
								{
									glm::vec2 data = (fieldExists ? instanceField.GetValue<glm::vec2>() : field.GetValue<glm::vec2>());
									if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
										instanceField.SetValue<glm::vec2>(data);
								}
								break;
								case ScriptField::Type::Vector3:
								{
									glm::vec3 data = (fieldExists ? instanceField.GetValue<glm::vec3>() : field.GetValue<glm::vec3>());
									if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
										instanceField.SetValue<glm::vec3>(data);
								}
								break;
								case ScriptField::Type::Vector4:
								{
									glm::vec4 data = (fieldExists ? instanceField.GetValue<glm::vec4>() : field.GetValue<glm::vec4>());
									if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data)))
										instanceField.SetValue<glm::vec4>(data);
								}
								break;
								case ScriptField::Type::Entity:
									break;
								}
							}
						}
						else
						{
							Ref<ScriptInstance> instance = Scripting::GetEntityInstance(entityUUID);
							if (instance)
							{
								for (const auto& [name, field] : classFieldMaps)
								{
									ScriptField::Type type = field.GetType();
									switch (type)
									{
									case ScriptField::Type::None:
										break;
									case ScriptField::Type::Char:
										break;
									case ScriptField::Type::Int:
									{
										int data = instance->GetFieldValue<int>(name);
										if (ImGui::DragInt(name.c_str(), &data))
											instance->SetFieldValue<int>(name, data);
									}
									break;
									case ScriptField::Type::UInt:
									{
										int data = instance->GetFieldValue<uint32_t>(name);
										if (ImGui::DragInt(name.c_str(), &data))
											instance->SetFieldValue<uint32_t>(name, data);
									}
									break;
									case ScriptField::Type::Float:
									{
										float data = instance->GetFieldValue<float>(name);
										if (ImGui::DragFloat(name.c_str(), &data))
											instance->SetFieldValue<float>(name, data);
									}
									break;
									case ScriptField::Type::Byte:
										break;
									case ScriptField::Type::Bool:
									{
										bool data = instance->GetFieldValue<bool>(name);
										if (ImGui::Checkbox(name.c_str(), &data))
											instance->SetFieldValue<bool>(name, data);
									}
									break;
									case ScriptField::Type::Vector2:
									{
										glm::vec2 data = instance->GetFieldValue<glm::vec2>(name);
										if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
											instance->SetFieldValue<glm::vec2>(name, data);
									}
									break;
									case ScriptField::Type::Vector3:
									{
										glm::vec3 data = instance->GetFieldValue<glm::vec3>(name);
										if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
											instance->SetFieldValue<glm::vec3>(name, data);
									}
									break;
									case ScriptField::Type::Vector4:
									{
										glm::vec4 data = instance->GetFieldValue<glm::vec4>(name);
										if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data)))
											instance->SetFieldValue<glm::vec4>(name, data);
									}
									break;
									case ScriptField::Type::Entity:
										break;
									}
								}
							}
						}
					}
				}
				else
					ImGui::PopStyleColor();

			});
#pragma endregion

#pragma region Physics

		DrawComponent<Rigidbody2DComponent>(scene, "Rigidbody 2D", entity,
			[](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];

				if (ImGui::BeginCombo("Type", currentBodyTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							currentBodyTypeString = bodyTypeStrings[i];
							component.Type = (Rigidbody2DComponent::BodyType)i;
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			});

		DrawComponent<BoxCollider2DComponent>(scene, "Box Collider 2D", entity,
			[](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
				ImGui::DragFloat("Density", &component.Density);
				ImGui::DragFloat("Friction", &component.Friction);
				ImGui::DragFloat("Restitution", &component.Restitution);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold);

				ImGui::Checkbox("Show", &component.Show);

			});

		DrawComponent<CircleCollider2DComponent>(scene, "Circle Collider 2D", entity,
			[](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat("Radius", &component.Radius);
				ImGui::DragFloat("Density", &component.Density);
				ImGui::DragFloat("Friction", &component.Friction);
				ImGui::DragFloat("Restitution", &component.Restitution);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold);

				ImGui::Checkbox("Show", &component.Show);

			});
#pragma endregion

	}

}