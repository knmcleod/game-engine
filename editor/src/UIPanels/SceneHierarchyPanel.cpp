#include "SceneHierarchyPanel.h"

#include "GE/Scene/Components/Components.h"
#include "GE/Rendering/Textures/Texture.h"

#include <filesystem>

namespace GE
{
	extern const std::filesystem::path g_AssetsPath;

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

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		SetScene(scene);
	}

	void SceneHierarchyPanel::SetScene(const Ref<Scene>& scene)
	{
		m_Scene = scene;
		m_SelectedEntity = Entity();
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		{
			ImGui::Begin("Scene Hierarchy");

			m_Scene->m_Registry.each([&](auto entityID)
				{
					Entity entity{ entityID, m_Scene.get() };
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
					m_Scene->CreateEntity("Empty Entity");
				
				ImGui::EndPopup();
			}

			ImGui::End();
		}

		{
			ImGui::Begin("Properties");

			if (m_SelectedEntity != Entity())
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
			m_Scene->DestroyEntity(entity);
			if (m_SelectedEntity == entity)
				m_SelectedEntity = {};
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
				DrawAddComponent<SpriteRendererComponent>("Sprite Renderer", entity);
				DrawAddComponent<CircleRendererComponent>("Circle Renderer", entity);
				DrawAddComponent<NativeScriptComponent>("Native Script", entity);
				DrawAddComponent<Rigidbody2DComponent>("Rigidbody 2D", entity);
				DrawAddComponent<BoxCollider2DComponent>("Box Collider 2D", entity);
				DrawAddComponent<CircleCollider2DComponent>("Circle Collider 2D", entity);
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
				auto& camera = component.Camera;

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

				float size = camera.GetCameraFOV();
				if (ImGui::DragFloat("Size", &size))
					camera.SetCameraFOV(size);

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
				
				ImGui::Button("Texture");
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = std::filesystem::path(g_AssetsPath) / path;

						component.Texture = Texture2D::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
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

		DrawComponent<NativeScriptComponent>("Native Script", entity,
			[](auto& component)
			{
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