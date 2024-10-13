#include "SceneHierarchyPanel.h"

#include "../Application/Layer/EditorLayerStack.h"
#include "../AssetManager/EditorAssetManager.h"

#include "GE/Asset/Assets/Font/Font.h"
#include "GE/Asset/Assets/Textures/Texture.h"

#include "GE/Audio/AudioManager.h"

#include "GE/Core/Application/Application.h"
#include "GE/Core/FileSystem/FileSystem.h"

#include "GE/Project/Project.h"

#include "GE/Scripting/Scripting.h"

#include <filesystem>

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>

namespace GE
{
#pragma region ImGUIDraw

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction function)
	{
		if (entity.HasComponent<T>())
		{
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
				| ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_SpanAvailWidth;

			auto& component = entity.GetComponent<T>();

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::SameLine();
			if (ImGui::Button("+"))
				ImGui::OpenPopup("ComponentSettings");

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;
				ImGui::EndPopup();
			}

			if (open)
			{
				function(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	template<typename T>
	static void DrawAddComponent(const std::string& name, Entity entity)
	{
		if (!entity.HasComponent<T>() && ImGui::MenuItem(name.c_str()))
		{
			entity.AddComponent<T>();

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

	Entity SceneHierarchyPanel::GetSelectedEntity() const
	{
		if (Ref<Scene> runtimeScene = Project::GetRuntimeScene())
			return runtimeScene->GetEntityByUUID(m_SelectedEntityID);
		return {};
	}

	void SceneHierarchyPanel::SetSelected(UUID selectedEntity /*= 0*/)
	{
		m_SelectedEntityID = selectedEntity;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		{
			ImGui::Begin("Scene Hierarchy");

			if (Ref<Scene> runtimeScene = Project::GetRuntimeScene())
			{
				std::vector<Entity> entities = runtimeScene->GetAllEntitiesWith<IDComponent>();
				for (Entity entity : entities)
				{
					DrawEntity(entity);
				}
				entities.clear();
				entities = std::vector<Entity>();

				if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				{
					m_SelectedEntityID = 0;
				}

				//	Right-Click Blank space
				if (ImGui::BeginPopupContextWindow(0))
				{
					if (ImGui::MenuItem("Create New Entity"))
					{
						runtimeScene->CreateEntity("New Entity", 0);
					}

					ImGui::EndPopup();
				}
			}
			ImGui::End();
		}

		{
			ImGui::Begin("Properties");

			if (Ref<Scene> runtimeScene = Project::GetRuntimeScene())
			{
				if (Entity e = runtimeScene->GetEntityByUUID(m_SelectedEntityID))
					DrawComponents(e);
			}

			ImGui::End();
		}
	}

	void SceneHierarchyPanel::DrawEntity(Entity entity)
	{
		auto& name = entity.GetComponent<NameComponent>().Name;
		auto& id = entity.GetComponent<IDComponent>().ID;
		ImGuiTreeNodeFlags flags = ((m_SelectedEntityID == id) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectedEntityID = id;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		// Show child entities if present
		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			bool childOpened = ImGui::TreeNodeEx((void*)9817239, flags, name.c_str());
			if (childOpened)
				ImGui::TreePop();

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			if (m_SelectedEntityID == id)
				m_SelectedEntityID = 0;
			if (Ref<Scene> runtimeScene = Project::GetRuntimeScene())
				runtimeScene->DestroyEntity(entity);
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<NameComponent>())
		{
			auto& name = entity.GetComponent<NameComponent>().Name;

			DrawInputText("\n", name);
		}

		ImGui::SameLine();
		DrawPopup("AddComponent", entity, [](auto& entity)
			{
				DrawAddComponent<TransformComponent>("Transform", entity);

				DrawPopup("Audio", entity, [](auto& entity)
					{
						DrawAddComponent<AudioSourceComponent>("Audio Source", entity);
						DrawAddComponent<AudioListenerComponent>("Audio Listener", entity);
					});

				DrawPopup("Rendering", entity, [](auto& entity)
					{
						DrawAddComponent<CameraComponent>("Camera", entity);

						DrawAddComponent<SpriteRendererComponent>("Sprite Renderer", entity);
						DrawAddComponent<CircleRendererComponent>("Circle Renderer", entity);
						DrawAddComponent<TextRendererComponent>("Text Renderer", entity);
					});

				DrawPopup("Scripting", entity, [](auto& entity)
					{
						DrawAddComponent<NativeScriptComponent>("Native Script", entity);
						DrawAddComponent<ScriptComponent>("Script", entity);
					});

				DrawPopup("Physics", entity, [](auto& entity)
					{
						DrawAddComponent<Rigidbody2DComponent>("Rigidbody 2D", entity);
						DrawAddComponent<BoxCollider2DComponent>("Box Collider 2D", entity);
						DrawAddComponent<CircleCollider2DComponent>("Circle Collider 2D", entity);
					});
			});

		if (entity.HasComponent<TagComponent>())
		{
			auto& tc = entity.GetComponent<TagComponent>();
			std::string tagStr = Project::GetTagByKey(tc.ID);
			if (tagStr.empty())
				tagStr = std::string("None");

			ImGui::Text("Tag:");
			ImGui::SameLine();
			if (ImGui::BeginMenu(tagStr.c_str()))
			{
				for (const auto& [id, string] : Project::GetTags())
				{
					bool isSelected = (tc.ID == id);
					if (!string.empty() && ImGui::MenuItem(string.c_str(), nullptr, isSelected))
					{
						GE_TRACE("Selected Tag : {0}, ID : {1}", string.c_str(), id);
						if (isSelected) // Clear if already selected
							tc.ID = 0;
						else // Set if not
							tc.ID = id;
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

		if (entity.HasComponent<RenderComponent>())
		{
			Ref<EditorLayerStack> editorLayerStack = Application::GetLayerStack<EditorLayerStack>();
			auto& rc = entity.GetComponent<RenderComponent>();
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
				for (const auto& layer : Application::GetLayers())
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
						Application::SubmitToMainAppThread([newLayerStr = m_NewLayerStr]()
							{
								Ref<Layer> layer = CreateRef<Layer>();
								if (Application::GetLayerStack<EditorLayerStack>()->InsertLayer(layer, newLayerStr))
									GE_TRACE("Successfully added new Layer to EditorLayerStack.");
							});
						m_NewLayerStr.clear();
					}
				}
				ImGui::EndMenu();
			}
		}

		ImGui::Separator();
		DrawComponent<TransformComponent>("Transform", entity,
			[](auto& component)
			{
				ImGui::DragFloat3("Position", glm::value_ptr(component.Translation));
				ImGui::DragFloat3("Rotation", glm::value_ptr(component.Rotation));
				ImGui::DragFloat3("Scale", glm::value_ptr(component.Scale));
			});

		DrawComponent<AudioSourceComponent>("Audio Source", entity,
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

		DrawComponent<AudioListenerComponent>("Audio Listener", entity,
			[](auto& component)
			{
				ImGui::Text("Device", AudioManager::GetDeviceName());
			});

		DrawComponent<CameraComponent>("Camera", entity,
			[](CameraComponent& cc)
			{
				SceneCamera& camera = cc.ActiveCamera;

				ImGui::Checkbox("Primary", &cc.Primary);
				ImGui::Checkbox("Fixed Aspect Ratio", &cc.FixedAspectRatio);

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
							camera.SetProjectionType((Camera::ProjectionType)type);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				float fov = camera.GetFOV();
				if (ImGui::DragFloat("FOV", &fov))
					camera.SetFOV(fov);

				float nearClip = camera.GetNearClip();
				if (ImGui::DragFloat("Near Clip", &nearClip))
					camera.SetNearClip(nearClip);

				float farClip = camera.GetFarClip();
				if (ImGui::DragFloat("Far Clip", &farClip))
					camera.SetFarClip(farClip);

			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity,
			[](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

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
				ImGui::DragFloat("Tiling Factor", &component.TilingFactor);
			});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity,
			[](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
				ImGui::DragFloat("Thickness", &component.Thickness, 0.25f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.Fade, 0.25f, 0.0f, 1.0f);

			});

		DrawComponent<TextRendererComponent>("Text Renderer", entity,
			[](auto& component)
			{
				ImGui::ColorEdit4("Text Color", glm::value_ptr(component.TextColor));
				ImGui::ColorEdit4("Background Color", glm::value_ptr(component.BGColor));

				ImGui::DragFloat("Kerning", &component.KerningOffset);
				ImGui::DragFloat("Line Height Spacing", &component.LineHeightOffset);

				std::string& text = component.Text;

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				if (!text.empty())
					strcpy_s(buffer, sizeof(buffer), text.c_str());
				if (ImGui::InputTextMultiline("Text", buffer, sizeof(buffer)))
					text = std::string(buffer);

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
								component.AssetHandle = font->GetHandle();

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
								component.AssetHandle = asset->GetHandle();
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

		DrawComponent<NativeScriptComponent>("Native Script", entity,
			[](auto& component)
			{
			});

		DrawComponent<ScriptComponent>("Script", entity,
			[entity, scene = Project::GetRuntimeScene()](ScriptComponent& sc) mutable
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
						UUID entityUUID = entity.GetComponent<IDComponent>().ID;
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
							Ref<ScriptInstance> instance = Scripting::GetScriptInstance(entityUUID);
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

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity,
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

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity,
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

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity,
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
	}

}