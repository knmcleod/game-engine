#include "EditorLayer.h"

#include "../../AssetManager/EditorAssetManager.h"

#include <GE/Asset/RuntimeAssetManager.h>
#include <GE/Asset/Assets/Scene/Components/Components.h>

#include <GE/Core/Application/Application.h>

#include <GE/Project/Project.h>
#include <GE/Rendering/Renderer/Renderer.h>
#include <GE/Scripting/Scripting.h>

#include <GE/Core/Debug/ImGUI/ImGuiBuild.cpp>
#include <glm/gtc/type_ptr.hpp>

namespace GE
{
	/*
	* Copies EditorAssetManager.LoadedAssets & Adds them to New RuntimeAssetManager
	* Reinitializes & deserializes EditorAssetManager after complete
	*/
	static void SerializeRuntimeAssetManager()
	{
		const AssetMap assetMap = Project::GetAssetManager<EditorAssetManager>()->GetLoadedAssets();
		Ref<RuntimeAssetManager> ram = Project::NewAssetManager<RuntimeAssetManager>(assetMap);

		if (ram->SerializeAssets())
			GE_INFO("SerializeRuntimeAssetManager Successful");

		// Revert to EditorAssetManager using full DeserializeAssets process
		Project::NewAssetManager<EditorAssetManager>()->DeserializeAssets();
	}

	/*
	* Deserializes New RuntimeAssetManager
	* Reinitializes & deserializes EditorAssetManager after complete
	*/
	static void DeserializeRuntimeAssetManager()
	{
		if (Project::NewAssetManager<RuntimeAssetManager>()->DeserializeAssets())
		{
			GE_INFO("DeserializeRuntimeAssetManager Successful");
			for (const auto& [uuid, asset] : Project::GetAssetManager()->GetLoadedAssets())
				GE_TRACE("Asset : {0}, {1}", (uint64_t)uuid, AssetUtils::AssetTypeToString(asset->GetType()).c_str());
		}

		// Revert to EditorAssetManager using full DeserializeAssets process
		Project::NewAssetManager<EditorAssetManager>()->DeserializeAssets();
	}

	EditorLayer::EditorLayer(uint64_t id) : Layer(id),
		m_ImGUIViewportBounds{ { 0.0, 0.0 },{ 0.0, 0.0 } }, m_ImGUIViewport({ 0.0, 0.0 }),
		m_ImGUIMousePosition({ 0.0f, 0.0f })
	{
	}

	void EditorLayer::RenderEntity(Ref<Scene> scene, Entity entity, glm::vec3& translationOffset, glm::vec3& rotationOffset)
	{
		if (!scene || !entity)
			return;

		auto& ac = scene->GetComponent<ActiveComponent>(entity);
		auto& rc = scene->GetComponent<RenderComponent>(entity);
		if (rc.Rendered || (!ac.Active || ac.Hidden) || !rc.IDHandled(p_Config.ID))
			return; // Entity; 1: Shouldn't be rendered. 2: Should be active && visible. 3: Handled by EditorLayer::ID

		auto& trsc = scene->GetComponent<TransformComponent>(entity);
		// TODO : Add GameLayer rendering(?)

		// First, Render self/parent
		if (scene->HasComponent<GUIComponent>(entity))
		{
			auto& guiC = scene->GetComponent<GUIComponent>(entity);

			if (scene->HasComponent<GUICanvasComponent>(entity))
			{
				rc.Rendered = true;
			}

			if (scene->HasComponent<GUIImageComponent>(entity))
			{
				auto& guiIC = scene->GetComponent<GUIImageComponent>(entity);

				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiIC, entity);

				rc.Rendered = true;
			}

			if (scene->HasComponent<GUIButtonComponent>(entity))
			{
				auto& guiBC = scene->GetComponent<GUIButtonComponent>(entity);

				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiBC, guiC.CurrentState, entity);

				rc.Rendered = true;

			}

			if (scene->HasComponent<GUIInputFieldComponent>(entity))
			{
				auto& guiIFC = scene->GetComponent<GUIInputFieldComponent>(entity);

				if (guiIFC.FillBackground)
					guiIFC.TextSize = Renderer::GetFontTextSize(guiIFC);
				
				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiIFC, guiC.CurrentState, entity);

				rc.Rendered = true;

			}

			if (scene->HasComponent<GUISliderComponent>(entity))
			{
				auto& guiSC = scene->GetComponent<GUISliderComponent>(entity);

				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiSC, guiC.CurrentState, entity);

				rc.Rendered = true;

			}

			if (scene->HasComponent<GUICheckboxComponent>(entity))
			{
				auto& guiCB = scene->GetComponent<GUICheckboxComponent>(entity);

				Renderer::Draw(trsc.GetTransform(translationOffset, rotationOffset), trsc.GetPivot(), guiCB, guiC.CurrentState, entity);

				rc.Rendered = true;

			}
		}

		// Then, render children offset from self/parent
		auto& rsc = scene->GetComponent<RelationshipComponent>(entity);
		if (!rsc.GetChildren().empty())
		{
			translationOffset += trsc.Translation;
			rotationOffset += trsc.Rotation;
			if (scene->HasComponent<GUILayoutComponent>(entity))
			{
				auto& guiLOC = scene->GetComponent<GUILayoutComponent>(entity);
				glm::vec2 layoutOffset = guiLOC.StartingOffset;
				for (const UUID& childID : rsc.GetChildren())
				{
					Entity childEntity = scene->GetEntityByUUID(childID);
					auto& childTRSC = scene->GetComponent<TransformComponent>(childEntity);

					childTRSC.Translation = glm::vec3(layoutOffset, 0.0f);
					childTRSC.Scale = glm::vec3(guiLOC.ChildSize, 1.0f);

					RenderEntity(scene, childEntity, translationOffset, rotationOffset);
					layoutOffset += guiLOC.GetEntityOffset();
				}
			}
			else
			{
				for (const UUID& childID : rsc.GetChildren())
				{
					Entity childEntity = scene->GetEntityByUUID(childID);
					RenderEntity(scene, childEntity, translationOffset, rotationOffset);
				}
			}
		}
	}

	void EditorLayer::OnAttach(Ref<Scene> scene)
	{
		GE_PROFILE_FUNCTION();
		GE_INFO("EditorLayer::OnAttach Start.");

		Layer::OnAttach(scene);

		ImGUI_OnAttach();

		if (Ref<Framebuffer> fb = Application::GetFramebuffer())
			m_EditorCamera = new EditorCamera(45.0f, 0.1f, 100.0f, (float)fb->GetWidth() / (float)fb->GetHeight());

		if (Ref<EditorAssetManager> eam = Project::GetAssetManager<EditorAssetManager>())
		{
			if (Ref<Asset> editorIcon = eam->GetAsset("textures/EditorIcon.png"))
			{
				m_EditorIconHandle = editorIcon->GetHandle();
				Application::SetIcon(m_EditorIconHandle);
			}
			if (Ref<Asset> playButton = eam->GetAsset("textures/Play_Button.png"))
				m_PlayButtonHandle = playButton->GetHandle();
			if (Ref<Asset> pauseButton = eam->GetAsset("textures/Pause_Button.png"))
				m_PauseButtonHandle = pauseButton->GetHandle();
			if (Ref<Asset> stepButton = eam->GetAsset("textures/Step_Button.png"))
				m_StepButtonHandle = stepButton->GetHandle();
			if (Ref<Asset> stopButton = eam->GetAsset("textures/Stop_Button.png"))
				m_StopButtonHandle = stopButton->GetHandle();

			if(Ref<Asset> font = eam->GetAsset("fonts/arial.ttf"))
				m_FontHandle = font->GetHandle();
		}

		LoadScene();
		GE_INFO("EditorLayer::OnAttach Complete.");
	}

	void EditorLayer::OnDetach()
	{
		GE_PROFILE_FUNCTION();

		Layer::OnDetach();

		ImGUI_OnDetach();

		delete[] m_EditorCamera;

	}

	void EditorLayer::OnUpdate(Ref<Scene> scene, Timestep ts)
	{
		if (Ref<Framebuffer> fb = Application::GetFramebuffer())
		{
			fb->Resize((uint32_t)m_ImGUIViewport.x, (uint32_t)m_ImGUIViewport.y, m_ImGUIViewportBounds[0], m_ImGUIViewportBounds[1]);
			if (scene)
			{
				scene->OnResizeViewport(fb->GetWidth(), fb->GetHeight());
				if (scene->IsRunning())
					m_ShowMouse = !m_ImGUIViewportHovered;

				if (m_EditorCamera)
				{
					m_EditorCamera->SetViewport(fb->GetWidth(), fb->GetHeight());
					m_EditorCamera->OnUpdate(ts);
					const Camera* camera = &*m_EditorCamera;
					OnRender(scene, camera);
				}
			}
			
			Application::SubmitToMainAppThread([el = this]() 
				{
					el->ImGUI_Begin();
					el->ImGUI_Update();
					el->ImGUI_End();
				});
		}
	}

	void EditorLayer::OnRender(Ref<Scene> scene, const Camera*& camera)
	{
		if (!scene)
			return;

		if (m_UseEditorCamera && camera)
		{
			Layer::OnRender(scene, camera);

			Renderer::Open(camera);

			std::vector<Entity> entities = scene->GetAllRenderEntities(p_Config.ID);

			// Render Editor GUILayer - GUIComponents : Canvas, Button, Input Field, Slider
			if (Entity primaryCameraEntity = scene->GetPrimaryCameraEntity(p_Config.ID))
			{
				// Primary camera is used to size GUICanvasComponent outline in overlay mode
				// In world mode, size is determined by CanvasEntity TransformComponent.Scale
				// Nothing will(should) be rendered to primary camera.
				auto& pcCC = scene->GetComponent<CameraComponent>(primaryCameraEntity);
				auto& pcTRSC = scene->GetComponent<TransformComponent>(primaryCameraEntity);
				const SceneCamera& primaryCamera = pcCC.ActiveCamera;
				const float& aspectRatio = primaryCamera.GetAspectRatio();
				const float& fov = primaryCamera.GetFOV();

				// Find & Render Canvases
				for (Entity entity : entities)
				{
					if (!scene->HasComponent<GUICanvasComponent>(entity))
						continue;

					auto& canvasTRSC = scene->GetComponent<TransformComponent>(entity);
					auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);

					if (m_ShowMouse)
					{
						guiCC.ShowMouse = m_ShowMouse;
						Application::SetCursorMode(Input::CursorMode::Normal);
					}
					
					switch (guiCC.Mode)
					{
					case CanvasMode::Overlay: // GUI Components follow Active Camera
					{
						canvasTRSC.Translation = pcTRSC.Translation;
					}
					break;
					case CanvasMode::World: // GUI Components exist in world like any other Component
						break;
					}

					// Render GUICanvasComponent outline using primary cameras width & height
					Renderer::DrawRect(canvasTRSC.GetTransform(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(fov * aspectRatio, fov, 1.0)),
						canvasTRSC.GetPivot(), m_GUIViewportColor, entity);
					
					RenderEntity(scene, entity, glm::vec3(0.0f), glm::vec3(0.0f));
				}
			}

			// Render Editor GameLayer Helpers - Camera Viewports, Colliders & Pivots
			for (Entity entity : entities)
			{
				if (scene->HasComponent<TransformComponent>(entity))
				{
					auto& trsc = scene->GetComponent<TransformComponent>(entity);

					// Shows SceneCamera Viewport
					if (scene->HasComponent<CameraComponent>(entity))
					{
						auto& trsc = scene->GetComponent<TransformComponent>(entity);
						auto& cc = scene->GetComponent<CameraComponent>(entity);

						const SceneCamera& camera = cc.ActiveCamera;
						const float& aspectRatio = camera.GetAspectRatio();
						const float& fov = camera.GetFOV();
						Renderer::DrawRect(trsc.GetTransform(), trsc.GetPivot(), m_SceneCameraViewportColor, entity);
						// Offset translation by Camera Depth/Distance & scale by scaled viewport
						Renderer::DrawRect(trsc.GetTransform(glm::vec3(0.0, 0.0, camera.GetNearClip() - camera.GetFarClip()),
							glm::vec3(0.0f), glm::vec3(fov * aspectRatio, fov, 1.0)),
							trsc.GetPivot(), m_SceneCameraViewportColor, entity);

						// TODO : Add lines connecting rects(?)
					}

					if (m_ShowColliders)
					{
						if (scene->HasComponent<BoxCollider2DComponent>(entity))
						{
							auto& bc2D = scene->GetComponent<BoxCollider2DComponent>(entity);
							Renderer::DrawRect(trsc.GetTransform(glm::vec3(bc2D.Offset, 0.0025f), glm::vec3(bc2D.Size, 1.0f)),
								trsc.GetPivot(), m_ColliderColor, entity);
						}

						if (scene->HasComponent<CircleCollider2DComponent>(entity))
						{
							auto& cc2D = scene->GetComponent<CircleCollider2DComponent>(entity);
							Renderer::DrawSphere(trsc.GetTransform(glm::vec3(cc2D.Offset, 0.0025f), glm::vec3(cc2D.Radius * 2.0f, cc2D.Radius * 2.0f, 1.0f)),
								trsc.GetPivot(), cc2D.Radius, m_CircleColliderThickness, m_CircleColliderFade, m_ColliderColor, entity);
						}
					}

					if (m_ShowPivotPoints)
					{
						glm::vec3 translationOffset = trsc.GetOffsetTranslation();
						glm::vec3 rotationOffset = glm::vec3(0.0f);
						scene->GetTotalOffset(entity, translationOffset, rotationOffset);
						glm::mat4 transform = glm::translate(Renderer::IdentityMat4(), translationOffset + (trsc.GetPivotOffset() / 2.0f))
							* glm::scale(Renderer::IdentityMat4(), glm::vec3(m_PivotSize, m_PivotSize, 1.0));
						Renderer::DrawSphere(transform, trsc.GetPivot(), 0.5f, m_PivotThickness, m_PivotFade, m_PivotColor, entity);
					}
				}
			}

			entities.clear();
			entities = std::vector<Entity>();

			Renderer::Close();

			Renderer::Open(camera);

			// Outline Selected Entity
			if (m_ScenePanel)
			{
				Entity selectedEntity = scene->GetEntityByUUID(m_ScenePanel->m_SelectedEntityID);
				if (selectedEntity && scene->HasComponent<TransformComponent>(selectedEntity))
				{
					auto& selectedTRSC = scene->GetComponent<TransformComponent>(selectedEntity);
					glm::vec3 translationOffset = glm::vec3(0.0f);
					glm::vec3 rotationOffset = glm::vec3(0.0f);
					scene->GetTotalOffset(selectedEntity, translationOffset, rotationOffset);
					glm::vec3 scale = glm::vec3(1.0f);
					if (scene->HasComponent<GUIInputFieldComponent>(selectedEntity))
						scale = glm::vec3(scene->GetComponent<GUIInputFieldComponent>(selectedEntity).TextSize, 1.0f);
					Renderer::DrawRect(selectedTRSC.GetTransform(translationOffset, rotationOffset, scale), selectedTRSC.GetPivot(), m_ScenePanel->GetSelectedColor(), selectedEntity);
				}
			}

			// Outline Hovered Entity
			m_HoveredEntity = Application::GetHoveredEntity(m_ImGUIMousePosition);
			if (m_HoveredEntity && scene->HasComponent<TransformComponent>(m_HoveredEntity))
			{
				auto& hoveredTRSC = scene->GetComponent<TransformComponent>(m_HoveredEntity);
				glm::vec3 translationOffset = glm::vec3(0.0f);
				glm::vec3 rotationOffset = glm::vec3(0.0f);
				scene->GetTotalOffset(m_HoveredEntity, translationOffset, rotationOffset);
				glm::vec3 scale = glm::vec3(1.0f);
				if (scene->HasComponent<GUIInputFieldComponent>(m_HoveredEntity))
					scale = glm::vec3(scene->GetComponent<GUIInputFieldComponent>(m_HoveredEntity).TextSize, 1.0f);
				Renderer::DrawRect(hoveredTRSC.GetTransform(translationOffset, rotationOffset, scale), hoveredTRSC.GetPivot(), m_HoveredColor, m_HoveredEntity);
			}

			Renderer::Close();
		}
		else
		{
			// Render UI & Game Layers using Primary Camera in Scene.
			// Scene Physics/Scriping/Audio will not be updated.
			if (scene->IsStopped())
			{
				if (Entity primaryCameraEntity = scene->GetPrimaryCameraEntity(p_Config.ID))
				{
					auto& pcTRSC = scene->GetComponent<TransformComponent>(primaryCameraEntity);
					auto& pcCC = scene->GetComponent<CameraComponent>(primaryCameraEntity);

					// Update Camera
					pcCC.SetPosition(pcTRSC.Translation);
					pcCC.OnUpdate(Application::GetTimestep());

					// Render GameLayer
					const Camera* primaryCamera = &pcCC.ActiveCamera;
					const float& aspectRatio = primaryCamera->GetAspectRatio();
					const float& fov = primaryCamera->GetFOV();
					Layer::OnRender(scene, primaryCamera);

					// Render GUILayer
					Renderer::Open(primaryCamera);

					std::vector<Entity> entities = scene->GetAllEntitiesWith<GUICanvasComponent>();
					for (Entity entity : entities)
					{
						// Render GUICanvasComponent outline using primary cameras width & height
						auto& canvasTRSC = scene->GetComponent<TransformComponent>(entity);
						auto& guiCC = scene->GetComponent<GUICanvasComponent>(entity);
						Renderer::DrawRect(canvasTRSC.GetTransform(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(fov * aspectRatio, fov, 1.0)),
							canvasTRSC.GetPivot(), m_GUIViewportColor, entity);

						auto& ac = scene->GetComponent<ActiveComponent>(entity);
						auto& rsc = scene->GetComponent<RelationshipComponent>(entity);
						auto& rc = scene->GetComponent<RenderComponent>(entity);

						if (guiCC.ControlMouse)
						{
							Application::SetCursorMode(!m_ImGUIViewportHovered ? Input::CursorMode::Normal : Input::CursorMode::Disabled);
							guiCC.ShowMouse = !m_ImGUIViewportHovered;
						}

						switch (guiCC.Mode)
						{
						case CanvasMode::Overlay: // GUI Components follow Active Camera
						{
							canvasTRSC.Translation = pcTRSC.Translation;
						}
						break;
						case CanvasMode::World: // GUI Components exist in world like any other Component
							break;
						}
						
						RenderEntity(scene, entity, glm::vec3(0.0f), glm::vec3(0.0f));
					}

					entities.clear();
					entities = std::vector<Entity>();

					Renderer::Close();
				}
			}
		}
	}

	void EditorLayer::OnEvent(Event& e)
	{
		if (!m_ImGUIViewportHovered)
			ImGUI_OnEvent(e);

		if (m_ImGUIViewportHovered && m_UseEditorCamera && m_EditorCamera)
			m_EditorCamera->OnEvent(e);

		Layer::OnEvent(e);

	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Application::IsKeyPressed(Input::KEY_LEFT_CONTROL) || Application::IsKeyPressed(Input::KEY_RIGHT_CONTROL);
		bool shift = Application::IsKeyPressed(Input::KEY_LEFT_SHIFT) || Application::IsKeyPressed(Input::KEY_RIGHT_SHIFT);

		Ref<Scene> scene = Project::GetRuntimeScene();
		if (scene && m_ScenePanel)
		{
			switch (e.GetKeyCode())
			{
			case Input::KEY_D:
			{
				if (control)
				{
					if (Entity entity = scene->GetEntityByUUID(m_ScenePanel->m_SelectedEntityID))
					{
						if (scene->EntityExists(entity))
						{
							scene->DuplicateEntity(entity);
						}
					}
				}
			}
				break;
			case Input::KEY_E:
			{
				if (control)
				{
					if (shift)
						SerializeRuntimeAssetManager();
				}
			}
				break;
			case Input::KEY_F:
			{
				if (control)
				{
					Application::SubmitToMainAppThread([isFullscreen = Application::IsFullscreen()]()
						{
							Application::SetFullscreen(!isFullscreen);
						});
				}
				else if (scene->IsStopped() && m_UseEditorCamera)
				{
					if (Entity entity = scene->GetEntityByUUID(m_ScenePanel->m_SelectedEntityID))
					{
						glm::vec3 translationOffset = glm::vec3(0.0f);
						glm::vec3 rotationOffset = glm::vec3(0.0f);
						scene->GetTotalOffset(entity, translationOffset, rotationOffset);
						m_EditorCamera->m_FocalPoint = translationOffset + scene->GetComponent<TransformComponent>(entity).GetOffsetTranslation();
						// TODO : Set EditorCamera Rotation
					}
				}
			}
				break;
			case Input::KEY_I:
			{
				if (control)
				{
					if (shift)
						DeserializeRuntimeAssetManager();
				}
			}
				break;
			case Input::KEY_O:
			{
				if (control)
				{
					if (shift)
						Application::LoadAppProjectFileDialog();
				}
			}
				break;
			case Input::KEY_S:
			{
				if (control)
				{
					if (shift)
						Application::SaveAppProjectFileDialog();
					else
						Application::SaveAppProject();
				}
			}
				break;
			case Input::KEY_R:
			{
				if (control)
				{
					Scripting::ReloadAssembly();
				}
			}
				break;
			case Input::KEY_DELETE:
			{
				// Not hovering ImGui, hovering Framebuffer/Scene
				if (ImGUI_WidgetID() == 0)
				{
					if (Entity selectedEntity = scene->GetEntityByUUID(m_ScenePanel->m_SelectedEntityID))
					{
						m_ScenePanel->ClearSelected();
						scene->DestroyEntity(selectedEntity);
					}
				}
			}
				break;
			case Input::KEY_ESCAPE:
			{
				Application::SubmitToMainAppThread([]() 
					{
						Application::SetFullscreen(false);
					});
			}
				break;
			default:
				break;
			}
		}
		return false;
	}

	bool EditorLayer::OnMousePressed(MousePressedEvent& e)
	{
		Ref<Scene> scene = Project::GetRuntimeScene();
		if (m_ImGUIViewportHovered && scene && m_ScenePanel)
		{
			switch (e.GetButton())
			{
			case Input::MOUSE_BUTTON_1:
				if (m_HoveredEntity && scene->HasComponent<IDComponent>(m_HoveredEntity))
				{
					m_ScenePanel->SetSelected(scene->GetComponent<IDComponent>(m_HoveredEntity).ID);
				}
				break;
			case Input::MOUSE_BUTTON_2:
				break;
			default:
				break;
			}
		}
		return false;
	}

#pragma region Scene Functions

	void EditorLayer::StartScene(const Scene::State& state)
	{
		if (state == Scene::State::Stop)
		{
			StopScene();
			return;
		}

		if (state == Scene::State::Run)
		{
			m_UseEditorCamera = false;
			m_ShowMouse = false;
		}

		UUID selectedID = 0;
		if (m_ScenePanel)
			m_ScenePanel->m_SelectedEntityID;

		Project::StartScene(state);

		if (m_ScenePanel)
			m_ScenePanel->SetSelected(selectedID);

	}

	void EditorLayer::StopScene()
	{
		m_UseEditorCamera = true;
		m_ShowMouse = true;
		Project::StopScene();

		// Old SelectedEntity.UUID won't exist after setting scene=editorScene
		// Save a copy to revert to after
		UUID selectedID = 0;
		if(m_ScenePanel)
			m_ScenePanel->m_SelectedEntityID;

		// Revert Scene
		if (Project::ResetScene())
		{
			GE_INFO("EditorLayer::StopScene() - Successfully Reset Scene.");
		}

		if (m_ScenePanel)
			m_ScenePanel->SetSelected(selectedID);
	}

	void EditorLayer::LoadScene(UUID handle)
	{
		if(handle != 0)
			Project::SetSceneHandle(handle);
		if (Ref<Scene> scene = Project::ResetScene())
		{
			m_AssetPanel = CreateRef<AssetPanel>();
			m_ScenePanel = CreateRef<SceneHierarchyPanel>();

			std::string sceneName = std::string("None");
			UUID sceneHandle = scene->GetHandle();
			if (Ref<EditorAssetManager> eam = Project::GetAssetManager<EditorAssetManager>())
			{
				if (eam->HandleExists(sceneHandle))
				{
					const AssetMetadata& sceneMetadata = eam->GetMetadata(sceneHandle);
					sceneName = sceneMetadata.FilePath.filename().string();
				}
			}
			GE_INFO("Editor Loaded Scene : {0}, {1}", sceneName.c_str(), (uint64_t)sceneHandle);
		}
	}

#pragma endregion

#pragma region ImGui Functions

	static void ImGUI_SetDarkTheme(ImGuiStyle& style)
	{
		auto& colors = style.Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.095f, 0.095f, 0.095f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.55f, 0.55f, 0.55f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.55f, 0.55f, 0.55f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.55f, 0.55f, 0.55f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

		// Tab
		colors[ImGuiCol_Tab] = ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.55f, 0.55f, 0.55f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.45f, 0.45f, 0.45f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

		// Name
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.55f, 0.55f, 0.55f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };
	}

	uint32_t EditorLayer::ImGUI_WidgetID() const
	{
		ImGuiContext& imguiContext = *GImGui;
		return imguiContext.ActiveId;
	}

	void EditorLayer::ImGUI_OnAttach()
	{
		GE_PROFILE_FUNCTION();
		//	Context Setup
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		//	Style Setup
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			GE::ImGUI_SetDarkTheme(style);
		}

		auto window = static_cast<GLFWwindow*>(Application::GetNativeWindow());

		//	Platform/Renderer bindings Setup
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void EditorLayer::ImGUI_OnDetach()
	{
		GE_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void EditorLayer::ImGUI_OnEvent(Event& e) const
	{
		if (m_ImGUIBlockEvents && !m_ImGUIViewportHovered)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (!e.IsHandled())
			{
				e.SetHandled(false | io.WantCaptureMouse);
				e.SetHandled(false | io.WantCaptureKeyboard);
			}
		}
	}

	void EditorLayer::ImGUI_Begin()
	{
		GE_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void EditorLayer::ImGUI_End()
	{
		GE_PROFILE_FUNCTION();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)Application::GetWindowWidth(), (float)Application::GetWindowHeight());

		//Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_contect = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_contect);
		}
	}

	void EditorLayer::ImGUI_Update()
	{
		GE_PROFILE_FUNCTION();
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable,
		// because it would be confusing to have two docking targets within each other.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse 
				| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, 
		// DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags))
		{
			ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImVec2 dockspaceSize = ImVec2(0.0f, 0.0f);
				ImGui::DockSpace(dockspace_id, dockspaceSize, dockspace_flags);
			}

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::BeginMenu("Project"))
					{
						if (ImGui::MenuItem("Save", "Ctrl+Shift+S"))
						{
							Application::SubmitToMainAppThread([]()
								{
									Application::SaveAppProject();
								});
						}
						if (ImGui::MenuItem("Load", "Ctrl+Shift+O")) Application::LoadAppProjectFileDialog();
						if (ImGui::MenuItem("Export", "Ctrl+Shift+E")) SerializeRuntimeAssetManager();
						if (ImGui::MenuItem("Import", "Ctrl+Shift+I")) DeserializeRuntimeAssetManager();

						ImGui::EndMenu();
					}

					ImGui::Separator();
					if (ImGui::MenuItem("Exit")) Application::CloseApp();
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Scripting"))
				{
					if (ImGui::MenuItem("Reload", "Ctrl+R")) Scripting::ReloadAssembly();
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			if (Ref<Scene> scene = Project::GetRuntimeScene())
			{
				if (ImGui::Begin("EditorSettings"))
				{

					const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
						| ImGuiTreeNodeFlags_AllowItemOverlap
						| ImGuiTreeNodeFlags_Framed
						| ImGuiTreeNodeFlags_SpanAvailWidth;
					
					const std::string ecStr = std::string("EditorCamera");
					if (ImGui::TreeNodeEx((void*)(uint64_t)m_ECID, treeNodeFlags, ecStr.c_str()))
					{
						ImGui::Checkbox("Toggle", &m_UseEditorCamera);
						if (m_EditorCamera)
						{
							ImGui::DragFloat3("Focal Point", glm::value_ptr(m_EditorCamera->m_FocalPoint));
							glm::vec3 rotation = glm::vec3(m_EditorCamera->p_Pitch, m_EditorCamera->p_Yaw, m_EditorCamera->p_Roll);
							ImGui::DragFloat3("Rotation", glm::value_ptr(rotation));
							glm::vec2 viewport = m_EditorCamera->GetViewport();
							if (ImGui::DragFloat2("Viewport", glm::value_ptr(viewport)))
								m_EditorCamera->SetViewport((uint32_t)viewport.x, (uint32_t)viewport.y);
							ImGui::DragFloat("Distance", &m_EditorCamera->m_Distance);

							ImGui::DragFloat("Aspect Ratio", &m_EditorCamera->p_AspectRatio);
							const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
							const char* currentProjectionTypeString = projectionTypeStrings[(int)m_EditorCamera->GetProjectionType()];
							if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
							{
								for (int type = 0; type < 2; type++)
								{
									bool isSelected = currentProjectionTypeString == projectionTypeStrings[type];
									if (ImGui::Selectable(projectionTypeStrings[type], isSelected))
									{
										currentProjectionTypeString = projectionTypeStrings[type];
										m_EditorCamera->SetProjectionType((Camera::ProjectionType)type);
									}

									if (isSelected)
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}

							float fov = m_EditorCamera->GetFOV();
							if (ImGui::DragFloat("FOV", &fov))
								m_EditorCamera->SetFOV(fov);

							float nearClip = m_EditorCamera->GetNearClip();
							if (ImGui::DragFloat("Near Clip", &nearClip))
								m_EditorCamera->SetNearClip(nearClip);

							float farClip = m_EditorCamera->GetFarClip();
							if (ImGui::DragFloat("Far Clip", &farClip))
								m_EditorCamera->SetFarClip(farClip);

						}
						ImGui::DragFloat4("Camera Viewport Color", glm::value_ptr(m_SceneCameraViewportColor));
						ImGui::DragFloat4("GUI Viewport Color", glm::value_ptr(m_GUIViewportColor));

						ImGui::TreePop();
					}

					ImGui::Separator();

					const std::string rsStr = std::string("Renderer");
					if (ImGui::TreeNodeEx((void*)(uint64_t)m_RSID, treeNodeFlags, rsStr.c_str()))
					{
						const Renderer::Statistics& stats = Renderer::GetStatistics();
						ImGui::Text("\tDraw Calls - %d", stats.DrawCalls);
						ImGui::Text("\tSpawn Count - %d", stats.SpawnCount);
						ImGui::Text("\tVertices - %d", stats.GetTotalVertexCount());
						ImGui::Text("\tIndices - %d", stats.GetTotalIndexCount());

						ImGui::TreePop();
					}

					ImGui::Separator();

					const std::string sStr = std::string("Scene");
					if (ImGui::TreeNodeEx((void*)(uint64_t)m_SID, treeNodeFlags, sStr.c_str()))
					{
						ImGui::DragInt("Step Rate", &m_StepFrameMultiplier);
						ImGui::Separator();
						{
							const std::string str = std::string("Pivot");
							if (ImGui::TreeNodeEx((void*)(uint64_t)m_PID, treeNodeFlags, str.c_str()))
							{
								ImGui::Checkbox("Show", &m_ShowPivotPoints);
								ImGui::DragFloat4("Color", glm::value_ptr(m_PivotColor));
								ImGui::DragFloat("Size", &m_PivotSize);
								ImGui::DragFloat("Thickness", &m_PivotThickness);
								ImGui::DragFloat("Fade", &m_PivotFade);

								ImGui::TreePop();
							}
						}
						ImGui::Separator();
						{
							const std::string str = std::string("Colliders");
							if (ImGui::TreeNodeEx((void*)(uint64_t)m_CID, treeNodeFlags, str.c_str()))
							{
								ImGui::Checkbox("Show", &m_ShowColliders);
								ImGui::DragFloat4("Color", glm::value_ptr(m_ColliderColor));
								ImGui::DragFloat("Thickness", &m_CircleColliderThickness);
								ImGui::DragFloat("Fade", &m_CircleColliderFade);
								ImGui::TreePop();
							}
						}
						ImGui::Separator();
						std::string name = "None";
						if (m_HoveredEntity && scene->HasComponent<NameComponent>(m_HoveredEntity))
							name = scene->GetComponent<NameComponent>(m_HoveredEntity).Name;
						ImGui::Text("Hovered Entity - %s", name.c_str());
						ImGui::DragFloat4("Hovered Color", glm::value_ptr(m_HoveredColor));

						if (m_ScenePanel)
							ImGui::DragFloat4("Selected Color", glm::value_ptr(m_ScenePanel->m_SelectedColor));

						ImGui::TreePop();
					}
					
					ImGui::End();
				}

				if (m_ScenePanel)
					m_ScenePanel->OnImGuiRender(scene);

				if (m_AssetPanel)
					m_AssetPanel->OnImGuiRender();

				// Scene Play/Pause/Stop Controls
				ImGUI_SceneToolbar(scene);

				// Framebuffer Viewport
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
				if (ImGui::Begin("Viewport"))
				{
					// Gather for next GE Update
					auto viewportOffset = ImGui::GetWindowPos();
					auto viewportMin = ImGui::GetWindowContentRegionMin();
					auto viewportMax = ImGui::GetWindowContentRegionMax();
					m_ImGUIViewportBounds[0] = { viewportMin.x + viewportOffset.x, viewportMin.y + viewportOffset.y };
					m_ImGUIViewportBounds[1] = { viewportMax.x + viewportOffset.x, viewportMax.y + viewportOffset.y };

					m_ImGUIViewportFocused = ImGui::IsWindowFocused();
					m_ImGUIViewportHovered = ImGui::IsWindowHovered();

					ImGUI_BlockEvents(!m_ImGUIViewportHovered);

					ImVec2 viewportSize = ImGui::GetContentRegionAvail();
					m_ImGUIViewport = { viewportSize.x, viewportSize.y };

					auto [mx, my] = ImGui::GetMousePos();
					m_ImGUIMousePosition = glm::vec2(mx, my);

					// Render framebuffer in dockspace if present
					if (Ref<Framebuffer> fb = Application::GetFramebuffer())
					{
						ImGui::Image((ImTextureID)(uint64_t)fb->GetAttachmentID(Framebuffer::Attachment::RGBA8), viewportSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
							{
								const UUID handle = *(UUID*)payload->Data;

								Ref<Asset> asset = Project::GetAssetManager<EditorAssetManager>()->GetAsset(handle);
								if (asset)
								{
									if (asset->GetType() == Asset::Type::Scene)
									{
										LoadScene(handle);
									}
									else
									{
										GE_WARN("Asset Type is not Scene.\n\tType:{0}", AssetUtils::AssetTypeToString(asset->GetType()));
									}
								}
								else
								{
									GE_ERROR("Could find Asset in AssetManager.");
								}
							}
							ImGui::EndDragDropTarget();
						}

					}

					ImGui::End();
				}
				ImGui::PopStyleVar();
			}

			ImGui::End();
		}
	}

	void EditorLayer::ImGUI_SceneToolbar(Ref<Scene> scene)
	{
		ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

		if (ImGui::Begin("Scene Toolbar", nullptr, toolbarFlags) && scene)
		{
			// Stop <-> Run
			if (!scene->IsPaused())
			{
				const Scene::State& currentState = scene->GetState();

				// Scene Runtime Start & Stop
				Scene::State handledState = Scene::State::Run;
				Ref<Texture2D> playStopButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_StopButtonHandle : m_PlayButtonHandle);

				ImGui::SameLine();
				if (playStopButtonTexture && ImGui::ImageButton((ImTextureID)(uint64_t)playStopButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
				{
					(currentState == handledState) ? StopScene() : StartScene(handledState);
				}
			}

			// Pause <-> Run
			if (!scene->IsStopped())
			{
				const Scene::State& currentState = scene->GetState();

				// Run
				Scene::State handledState = Scene::State::Run;

				Ref<Texture2D> playPauseButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_PauseButtonHandle : m_PlayButtonHandle);

				ImGui::SameLine();
				if (playPauseButtonTexture && ImGui::ImageButton((ImTextureID)(uint64_t)playPauseButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
				{
					(currentState == handledState) ? StartScene(Scene::State::Pause) : StartScene(handledState);
				}

				// Step Pause
				if (scene->IsPaused())
				{
					Ref<Texture2D> stepButtonTexture = Project::GetAsset<Texture2D>(m_StepButtonHandle);
					ImGui::SameLine();
					if (stepButtonTexture && ImGui::ImageButton((ImTextureID)(uint64_t)stepButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
						Project::StepScene(m_StepFrameMultiplier); // Adds step frames to queue. Handled in Scene::OnPauseUpdate.
				}

			}

			ImGui::End();
		}
	}

#pragma endregion

}