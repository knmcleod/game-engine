#include "SceneHierarchyPanel.h"

#include "../AssetManager/EditorAssetManager.h"

#include "GE/Audio/AudioManager.h"
#include "GE/Asset/Assets/Textures/Texture.h"

#include "GE/Core/FileSystem/FileSystem.h"

#include "GE/Project/Project.h"

#include "GE/Scripting/Scripting.h"

#include <filesystem>

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

namespace GE
{
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

	template<typename UIFunction>
	static void DrawPopup(const std::string& name, Entity entity, UIFunction function)
	{
		if (ImGui::Button(name.c_str()))
			ImGui::OpenPopup(name.c_str());

		if (ImGui::BeginPopup(name.c_str()))
		{
			function(entity);
			ImGui::EndPopup();
		}
	}

	SceneHierarchyPanel::SceneHierarchyPanel(Scene* scene)
	{
		SetScene(scene);
		
	}

	void SceneHierarchyPanel::SetScene(Scene* scene, UUID selectedEntity /*= 0*/)
	{
		m_Scene = scene;
		if (m_SelectedEntity != Entity() && m_SelectedEntity.HasComponent<IDComponent>())
			m_SelectedEntity = m_Scene->GetEntityByUUID(m_SelectedEntity.GetComponent<IDComponent>().ID);
		else if (selectedEntity != 0)
			m_SelectedEntity = m_Scene->GetEntityByUUID(selectedEntity);
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		{
			ImGui::Begin("Scene Hierarchy");

			m_Scene->GetRegistry().each([&](auto entityID)
				{
					Entity entity{ entityID, m_Scene };
					DrawEntity(entity);
				});

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				m_SelectedEntity = Entity();
			}
			
			//	Right-Click Blank space
			if (ImGui::BeginPopupContextWindow(0))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
				{
					m_Scene->CreateEntity("Empty Entity");
				}
				
				ImGui::EndPopup();
			}

			ImGui::End();
		}

		{
			ImGui::Begin("Properties");

			if (m_SelectedEntity)
			{
				DrawComponents(m_SelectedEntity);
			}

			ImGui::End();
		}
	}

	void SceneHierarchyPanel::DrawEntity(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
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
			bool childOpened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (childOpened)
				ImGui::TreePop();

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			if (m_SelectedEntity == entity)
				m_SelectedEntity = Entity();
			m_Scene->DestroyEntity(entity);
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if(entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
				tag = std::string(buffer);
			
		}
		
		ImGui::SameLine();
		DrawPopup("AddComponent", entity, [](auto& entity)
			{
				DrawAddComponent<TransformComponent>("Transform", entity);
				DrawAddComponent<CameraComponent>("Camera", entity);
				
				DrawPopup("Audio", entity, [](auto& entity)
					{
						DrawAddComponent<AudioSourceComponent>("Audio Source", entity);
						DrawAddComponent<AudioListenerComponent>("Audio Listener", entity);
					});

				DrawPopup("Rendering", entity, [](auto& entity)
					{
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

		DrawComponent<TransformComponent>("Transform", entity,
			[](auto& component)
			{
				ImGui::DragFloat3("Position", glm::value_ptr(component.Translation));
				ImGui::DragFloat3("Rotation", glm::value_ptr(component.Rotation));
				ImGui::DragFloat3("Scale", glm::value_ptr(component.Scale));
			});

		DrawComponent<CameraComponent>("Camera", entity,
			[](auto& component)
			{
				auto& camera = component.ActiveCamera;

				ImGui::Checkbox("Primary", &component.Primary);
				ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);

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
				if(!text.empty())
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
								GE_WARN("Asset Type is not Texture2D.");
							}
						}
						ImGui::EndDragDropTarget();
					}
				
					if (font)
					{
						Ref<Texture2D> fontAtlas = Project::GetAsset<Font>(font->GetHandle())->GetAtlasTexture();
						if (fontAtlas != nullptr)
							ImGui::Image((ImTextureID)fontAtlas->GetID(), { 512, 512 }, { 0, 1 }, { 1, 0 });
					}
				}

			});

		DrawComponent<NativeScriptComponent>("Native Script", entity,
			[](auto& component)
			{
			});

		DrawComponent<ScriptComponent>("Script", entity,
			[entity, scene = m_Scene](auto& component) mutable
			{
				bool exists = Scripting::ScriptClassExists(component.ClassName);
				if (!exists)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0));

				Buffer buffer = Buffer(sizeof(std::string));
				strcpy_s(buffer.As<char>(), buffer.GetSize(), component.ClassName.c_str());
				if (ImGui::InputText("Class", buffer.As<char>(), sizeof(buffer)))
					component.ClassName = std::string(buffer.As<char>());
				buffer.Release();

				// Script Fields
				if (exists)
				{
					if (scene->IsRunning())
					{
						UUID id = scene->GetComponent<IDComponent>(entity).ID;
						Ref<ScriptInstance> scriptInstance = Scripting::GetScriptInstance(id);
						if (scriptInstance)
						{
							const auto& fields = scriptInstance->GetScriptClass()->GetFields();

							for (const auto& [name, field] : fields)
							{
								ScriptField::Type type = field.GetType();
								if (type == ScriptField::Type::Bool)
								{
									bool data = scriptInstance->GetFieldValue<bool>(name);
									if (ImGui::Checkbox(name.c_str(), &data))
										scriptInstance->SetFieldValue<bool>(name, data);
								}
								else if (type == ScriptField::Type::Float)
								{
									float data = scriptInstance->GetFieldValue<float>(name);
									if (ImGui::DragFloat(name.c_str(), &data))
										scriptInstance->SetFieldValue<float>(name, data);
								}
								else if (type == ScriptField::Type::Vector2)
								{
									glm::vec2 data = scriptInstance->GetFieldValue<glm::vec2>(name);
									if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
										scriptInstance->SetFieldValue<glm::vec2>(name, data);
								}
								else if (type == ScriptField::Type::Vector3)
								{
									glm::vec3 data = scriptInstance->GetFieldValue<glm::vec3>(name);
									if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
										scriptInstance->SetFieldValue<glm::vec3>(name, data);
								}
								else if (type == ScriptField::Type::Vector4)
								{
									glm::vec4 data = scriptInstance->GetFieldValue<glm::vec4>(name);
									if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data)))
										scriptInstance->SetFieldValue<glm::vec4>(name, data);
								}

							}
						}
					}
					else
					{
						Ref<ScriptClass> scriptClass = Scripting::GetScriptClass(component.ClassName);
						const auto& fields = scriptClass->GetFields();
						auto& scriptingFieldMap = Scripting::GetScriptFieldMap(entity);

						for (const auto& [name, field] : fields)
						{
							// ScriptFieldInstance already exists, display it
							if (scriptingFieldMap.find(name) != scriptingFieldMap.end())
							{
								ScriptField& field = scriptingFieldMap.at(name);
								ScriptField::Type type = field.GetType();
								if (type == ScriptField::Type::Bool)
								{
									bool data = field.GetValue<bool>();
									if (ImGui::Checkbox(name.c_str(), &data))
										field.SetValue<bool>(data);
								}
								else if (type == ScriptField::Type::Float)
								{
									float data = field.GetValue<float>();
									if (ImGui::DragFloat(name.c_str(), &data))
										field.SetValue<float>(data);
								}
								else if (type == ScriptField::Type::Vector2)
								{
									glm::vec2 data = field.GetValue<glm::vec2>();
									if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
										field.SetValue<glm::vec2>(data);
								}
								else if (type == ScriptField::Type::Vector3)
								{
									glm::vec3 data = field.GetValue<glm::vec3>();
									if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
										field.SetValue<glm::vec3>(data);
								}
								else if (type == ScriptField::Type::Vector4)
								{
									glm::vec4 data = field.GetValue<glm::vec4>();
									if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data)))
										field.SetValue<glm::vec4>(data);
								}

							}
							else // Add ScriptFieldInstance for display
							{
								ScriptField& field = scriptingFieldMap[name];
								ScriptField::Type type = field.GetType();
								if (type == ScriptField::Type::Bool)
								{
									bool data = false;
									if (ImGui::Checkbox(name.c_str(), &data))
										field.SetValue<bool>(data);
								}
								else if (type == ScriptField::Type::Float)
								{
									float data = 0.0f;
									if (ImGui::DragFloat(name.c_str(), &data))
										field.SetValue<float>(data);
								}
								else if (type == ScriptField::Type::Vector2)
								{
									glm::vec2 data = glm::vec2();
									if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
										field.SetValue<glm::vec2>(data);
								}
								else if (type == ScriptField::Type::Vector3)
								{
									glm::vec3 data = glm::vec3();
									if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
										field.SetValue<glm::vec3>(data);
								}
								else if (type == ScriptField::Type::Vector4)
								{
									glm::vec4 data = glm::vec4();
									if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data)))
										field.SetValue<glm::vec4>(data);
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
				const char* bodyTypeStrings[] = {"Static", "Dynamic", "Kinematic"};
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