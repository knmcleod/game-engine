#include "EditorLayer.h"

#include "../../AssetManager/EditorAssetManager.h"

#include <GE/Asset/RuntimeAssetManager.h>
#include <GE/Asset/Assets/Scene/Components/Components.h>

#include <GE/Core/Application/Application.h>
#include <GE/Core/Input/Input.h>

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

	EditorLayer::EditorLayer() : EditorLayer(0, false)
	{
	}

	EditorLayer::EditorLayer(uint32_t id, bool isBase) : Layer(id, isBase),
		m_ImGUIViewportBounds{ { 0.0, 0.0 },{ 0.0, 0.0 } }, m_ImGUIViewport({ 0.0, 0.0 }),
		m_ImGUIMousePosition({ 0.0f, 0.0f })
	{
	}

	void EditorLayer::OnAttach()
	{
		GE_PROFILE_FUNCTION();
		GE_INFO("EditorLayer::OnAttach Start.");

		Layer::OnAttach();

		ImGUI_OnAttach();

		if (Ref<Framebuffer> fb = Application::GetFramebuffer())
			m_EditorCamera = new EditorCamera(45.0f, 0.1f, 100.0f, (float)fb->GetWidth() / (float)fb->GetHeight());

		m_PlayButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Play_Button.png")->GetHandle();
		m_PauseButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Pause_Button.png")->GetHandle();
		m_StepButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Step_Button.png")->GetHandle();
		m_StopButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Stop_Button.png")->GetHandle();

		m_FontHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("fonts/arial.ttf")->GetHandle();

		LoadScene();

		GE_INFO("EditorLayer::OnAttach Complete.");
	}

	void EditorLayer::OnDetach()
	{
		GE_PROFILE_FUNCTION();

		Layer::OnDetach();

		delete[] m_EditorCamera;

		ImGUI_OnDetach();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		if (Ref<Framebuffer> fb = Application::GetFramebuffer())
		{
			fb->Resize((uint32_t)m_ImGUIViewport.x, (uint32_t)m_ImGUIViewport.y, m_ImGUIViewportBounds[0], m_ImGUIViewportBounds[1]);

			if (m_UseEditorCamera && m_EditorCamera)
			{
				m_EditorCamera->SetViewport(fb->GetWidth(), fb->GetHeight());
				m_EditorCamera->OnUpdate(ts);
				OnRender(m_EditorCamera);
			}

			/*
			* Updates ImGui - Renders SceneHierarchy/Asset Panel & Framebuffer to viewport
			*/
			Application::SubmitToMainAppThread([editorLayer = this]()
				{
					GE_PROFILE_SCOPE("ImGui Render");
					editorLayer->ImGUI_Begin();
					editorLayer->ImGUI_Render();
					editorLayer->ImGUI_End();
				}); 
		}

	}

	void EditorLayer::OnRender(Camera* camera)
	{
		if (m_UseEditorCamera && camera)
		{
			Layer::OnRender(camera);

			Renderer::Open(*camera);

			if (m_ShowColliders)
			{
				std::vector<Entity> entities = Project::GetRuntimeScene()->GetAllEntitiesWith<RenderComponent>();
				for (Entity entity : entities)
				{
					if (Validate(entity) && entity.HasComponent<TransformComponent>())
					{
						auto& trsc = entity.GetComponent<TransformComponent>();

						if (entity.HasComponent<BoxCollider2DComponent>())
						{
							auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();
							Renderer::DrawRect(trsc.GetTransform(glm::vec3(bc2D.Offset, 0.0025f), glm::vec3(bc2D.Size * m_ColliderModifier, 1.0f)), m_ColliderColor, entity);
						}

						if (entity.HasComponent<CircleCollider2DComponent>())
						{
							auto& cc2D = entity.GetComponent<CircleCollider2DComponent>();
							Renderer::DrawSphere(trsc.GetTransform(glm::vec3(cc2D.Offset, 0.0025f), glm::vec3(glm::vec2(cc2D.Radius * (m_ColliderModifier * m_ColliderModifier)), 1.0f)), m_ColliderColor, cc2D.Radius, m_CircleColliderThickness, m_CircleColliderFade, entity);
						}
					}
				}
				entities.clear();
				entities = std::vector<Entity>();
			}

			m_HoveredEntity = Application::GetHoveredEntity(m_ImGUIMousePosition.x, m_ImGUIMousePosition.y, -1);
			// Outline Hovered Entity
			if (m_HoveredEntity && m_HoveredEntity.HasComponent<TransformComponent>())
			{
				TransformComponent tc = m_HoveredEntity.GetComponent<TransformComponent>();
				Renderer::DrawRect(tc.GetTransform(), m_HoveredColor, m_HoveredEntity);
			}

			// Outline Selected Entity
			if (m_ScenePanel)
			{
				const Entity& selectedEntity = m_ScenePanel->GetSelectedEntity();
				if (selectedEntity && selectedEntity.HasComponent<TransformComponent>())
				{
					TransformComponent tc = selectedEntity.GetComponent<TransformComponent>();
					Renderer::DrawRect(tc.GetTransform(), m_ScenePanel->GetSelectedColor(), selectedEntity);
				}
			}

			Renderer::Close();
		}
	}

	void EditorLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_EVENT_FN(OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(GE_BIND_EVENT_FN(OnMousePressed));

		if (m_ImGUIViewportFocused && m_ImGUIViewportHovered && m_UseEditorCamera && m_EditorCamera)
			m_EditorCamera->OnEvent(e);

		ImGUI_OnEvent(e);
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(Input::KEY_LEFT_CONTROL) || Input::IsKeyPressed(Input::KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(Input::KEY_LEFT_SHIFT) || Input::IsKeyPressed(Input::KEY_RIGHT_SHIFT);

		switch (e.GetKeyCode())
		{
		case Input::KEY_D:
		{
			if (control)
			{
				if (m_ScenePanel)
				{
					if(Entity e = m_ScenePanel->GetSelectedEntity())
						OnDuplicateEntity(e);
				}
			}
				
			break;
		}
		case Input::KEY_E:
		{
			if (control)
			{
				if (shift)
					SerializeRuntimeAssetManager();
			}
			break;
		}
		case Input::KEY_F:
		{
			if (control)
				Application::SetFullscreen(true);
			break;
		}
		case Input::KEY_I:
		{
			if (control)
			{
				if (shift)
					DeserializeRuntimeAssetManager();
			}
			break;
		}
		case Input::KEY_O:
		{
			if (control)
			{
				if (shift)
					Application::LoadAppProjectFileDialog();
			}
			break;
		}
		case Input::KEY_S:
		{
			if (control)
			{
				if (shift)
					Application::SaveAppProjectFileDialog();
				else
					Application::SaveAppProject();
			}

			break;
		}
		case Input::KEY_R:
		{
			if (control)
			{
				Scripting::ReloadAssembly();
			}

			break;
		}
		case Input::KEY_DELETE:
		{
			// Not hovering ImGui, hovering Framebuffer/Scene
			if (ImGUI_WidgetID() == 0)
			{
				Entity selectedEntity = m_ScenePanel->GetSelectedEntity();
				if (selectedEntity)
				{
					if (selectedEntity == m_ScenePanel->GetSelectedEntity())
						m_ScenePanel->ClearSelected();
					// If runtime, only destroys runtime Entity
					// if editor, destroys Entity in both
					Project::GetRuntimeScene()->DestroyEntity(selectedEntity);
				}
			}
			break;
		}
		case Input::KEY_ESCAPE:
		{
			Application::SetFullscreen(false);
			break;
		}
		default:
			break;
		}
		return true;
	}

	bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& e)
	{
		switch (e.GetMouseButton())
		{
		case Input::MOUSE_BUTTON_1:
			if (m_ImGUIViewportHovered && m_HoveredEntity && m_HoveredEntity.HasComponent<IDComponent>())
			{
				m_ScenePanel->SetSelected(m_HoveredEntity.GetComponent<IDComponent>().ID);
			}
			break;
		case Input::MOUSE_BUTTON_2:
			break;
		default:
			break;
		}
		return true;
	}

	void EditorLayer::OnDuplicateEntity(Entity e)
	{
		if (Ref<Scene> runtimeScene = Project::GetRuntimeScene())
		{
			if (runtimeScene->EntityExists(e))
				runtimeScene->DuplicateEntity(e);
		}
	}

	UUID EditorLayer::PopSelectedID()
	{
		UUID selectedUUID = 0;
		if (m_ScenePanel)
		{
			const Entity& entity = m_ScenePanel->GetSelectedEntity();
			if (entity && entity.HasComponent<IDComponent>())
			{
				selectedUUID = entity.GetComponent<IDComponent>().ID;
				m_ScenePanel->ClearSelected();
			}
		}
		return selectedUUID;
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
			m_UseEditorCamera = false;

		UUID selectedID = PopSelectedID();
		if (m_ScenePanel)
			m_ScenePanel->SetSelected(selectedID);

		Project::StartScene(state);

	}

	void EditorLayer::StopScene()
	{
		m_UseEditorCamera = true;

		// Old SelectedEntity.UUID won't exist after setting runtimeScene=editorScene
		// Save a copy to revert to after
		UUID selectedID = PopSelectedID();
		// Revert Scene
		if (Project::ResetScene() && m_ScenePanel)
		{
			m_ScenePanel->SetSelected(selectedID);
		}
	}

	void EditorLayer::LoadScene(UUID handle)
	{
		if(handle != 0)
			Project::SetSceneHandle(handle);
		if (Project::ResetScene())
		{
			if (Ref<Scene> scene = Project::GetRuntimeScene())
			{
				m_AssetPanel = CreateRef<AssetPanel>();
				m_ScenePanel = CreateRef<SceneHierarchyPanel>();

				std::string sceneName = std::string("None");
				if (Ref<EditorAssetManager> eam = Project::GetAssetManager<EditorAssetManager>())
				{
					const AssetMetadata& sceneMetadata = eam->GetMetadata(handle);
					sceneName = sceneMetadata.FilePath.filename().string();
				}
				GE_INFO("Editor Loaded Scene : {0}, {1}", sceneName.c_str(), (uint64_t)handle);
			}
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
			ImGUI_SetDarkTheme(style);
		}

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetNativeWindow());

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
		if (m_ImGUIBlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (!e.IsHandled())
			{
				e.SetHandled(false | e.IsInCategory(Event::Mouse) & io.WantCaptureMouse);
				e.SetHandled(false | e.IsInCategory(Event::Keyboard) & io.WantCaptureKeyboard);
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

	void EditorLayer::ImGUI_Render()
	{
		GE_PROFILE_FUNCTION();
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::BeginMenu("Project"))
				{
					if (ImGui::MenuItem("Save", "Ctrl+Shift+S")) Application::SaveAppProject();
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

		// Statistics
		{
			ImGui::Begin("Statistics");

			{
				Renderer::Statistics stats = Renderer::Get()->GetStats();
				ImGui::Text("Renderer");
				ImGui::Text("\tDraw Calls - %d", stats.DrawCalls);
				ImGui::Text("\tSpawn Count - %d", stats.SpawnCount);
				ImGui::Text("\tVertices - %d", stats.GetTotalVertexCount());
				ImGui::Text("\tIndices - %d", stats.GetTotalIndexCount());
			}

			ImGui::Separator();

			{
				ImGui::Text("Scene");
				ImGui::Checkbox("Editor Camera Toggle", &m_UseEditorCamera);
				ImGui::DragInt("Step Rate", &m_StepFrameMultiplier);
				ImGui::Separator();
				ImGui::Text("Colliders");
				ImGui::Checkbox("Show", &m_ShowColliders);
				ImGui::DragFloat4("Color", glm::value_ptr(m_ColliderColor));
				ImGui::DragFloat("Size Modifer", &m_ColliderModifier);
				ImGui::DragFloat("Circle Thickness", &m_CircleColliderThickness);
				ImGui::DragFloat("Circle Fade", &m_CircleColliderFade);
				ImGui::Separator();

				std::string name = "None";
				if (m_HoveredEntity && m_HoveredEntity.HasComponent<NameComponent>())
					name = m_HoveredEntity.GetComponent<NameComponent>().Name;
				ImGui::Text("Hovered Entity - %s", name.c_str());
				ImGui::DragFloat4("Hovered Color", glm::value_ptr(m_HoveredColor));

				if (m_ScenePanel)
					ImGui::DragFloat4("Selected Color", glm::value_ptr(m_ScenePanel->m_SelectedColor));
				
			}

			ImGui::End();
		}

		// Scene & Asset Panels
		{
			if (m_ScenePanel)
				m_ScenePanel->OnImGuiRender();

			if (m_AssetPanel)
				m_AssetPanel->OnImGuiRender();
		}

		// Framebuffer Viewport
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin("Viewport");

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
				ImGui::Image((ImTextureID)(uint64_t)fb->GetAttachmentID(Framebuffer::Attachment::RGBA8), viewportSize, ImVec2{0, 1}, ImVec2{1, 0});
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
			ImGui::PopStyleVar();

		}

		ImGUI_SceneToolbar();

		ImGui::End();
	}

	void EditorLayer::ImGUI_SceneToolbar()
	{
		ImGui::Begin("##UItoolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		// Scene State Control Buttons
		if(Ref<Scene> runtimeScene = Project::GetRuntimeScene())
		{
			// Stop <-> Run
			if (!runtimeScene->IsPaused())
			{
				const Scene::State& currentState = runtimeScene->GetState();

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
			if (!runtimeScene->IsStopped())
			{
				const Scene::State& currentState = runtimeScene->GetState();

				// Run
				Scene::State handledState = Scene::State::Run;

				Ref<Texture2D> playPauseButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_PauseButtonHandle : m_PlayButtonHandle);

				ImGui::SameLine();
				if (playPauseButtonTexture && ImGui::ImageButton((ImTextureID)(uint64_t)playPauseButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
				{
					(currentState == handledState) ? StartScene(Scene::State::Pause) : StartScene(handledState);
				}

				// Step Pause
				if (runtimeScene->IsPaused())
				{
					Ref<Texture2D> stepButtonTexture = Project::GetAsset<Texture2D>(m_StepButtonHandle);
					ImGui::SameLine();
					if (stepButtonTexture && ImGui::ImageButton((ImTextureID)(uint64_t)stepButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
						Project::StepScene(m_StepFrameMultiplier); // Adds step frames to queue. Handled in Scene::OnPauseUpdate.
				}

			}
		}

		ImGui::End();
		
	}

#pragma endregion

}