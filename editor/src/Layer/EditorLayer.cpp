#include "EditorLayer.h"

#include "../AssetManager/EditorAssetManager.h"

#include <GE/GE.h>

namespace GE
{
	static Ref<Font> s_font;

	EditorLayer::EditorLayer(const std::string& name)
		: Layer(name), m_ViewportSize(1.0f), m_ViewportBounds{ { glm::vec2() },{ glm::vec2() } }
	{
		s_font = Font::GetDefault();
	}

	void EditorLayer::OnAttach()
	{
		GE_PROFILE_FUNCTION();
		GE_INFO("Editor Layer OnAttach Start.");
		// Framebuffer
		FramebufferSpecification framebufferSpec;
		framebufferSpec.AttachmentSpecification = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH24STENCIL8 };
		framebufferSpec.Width = 1280;
		framebufferSpec.Height = 72;
		m_Framebuffer = Framebuffer::Create(framebufferSpec);

		m_EditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 100.0f);

		auto commandLineArgs = Application::GetApplication().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto projectFilePath = commandLineArgs[1];
			GE_INFO("Loading Project from commandLineArgs. Path = {}", projectFilePath);
			LoadProject(projectFilePath);
		}
		else
		{
			if (!LoadProjectFromFile())
				Application::GetApplication().Close();
		}

		Ref<Asset> playButtonTexture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(AssetMetadata("textures/Play_Button.png"));
		m_PlayButtonTexture = Project::GetAsset<Texture2D>(playButtonTexture->GetHandle());

		Ref<Asset> simulateButtonTexture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(AssetMetadata("textures/Simulate_Button.png"));
		m_SimulateButtonTexture = Project::GetAsset<Texture2D>(simulateButtonTexture->GetHandle()); 

		Ref<Asset> pauseButtonTexture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(AssetMetadata("textures/Pause_Button.png"));
		m_PauseButtonTexture = Project::GetAsset<Texture2D>(pauseButtonTexture->GetHandle());

		Ref<Asset> stepButtonTexture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(AssetMetadata("textures/Step_Button.png"));
		m_StepButtonTexture = Project::GetAsset<Texture2D>(stepButtonTexture->GetHandle());

		Ref<Asset> stopButtonTexture = Project::GetAssetManager<EditorAssetManager>()->GetAsset(AssetMetadata("textures/Stop_Button.png"));
		m_StopButtonTexture = Project::GetAsset<Texture2D>(stopButtonTexture->GetHandle());

		GE_INFO("Editor Layer OnAttach Complete.");
	}

	void EditorLayer::OnDetach()
	{
		GE_PROFILE_FUNCTION();

		Scripting::Shutdown();
	}

	void EditorLayer::OnUpdate(Timestep timestep)
	{
		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewport(m_ViewportSize.x, m_ViewportSize.y);
		}

		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
		RenderCommand::ClearAPI();

		// Clear attachmentIndex = 1 - RED_INTEGER / entityID
		m_Framebuffer->ClearAttachment(1, -1);

		if (m_ActiveScene)
		{
			m_ActiveScene->OnResizeViewport((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			switch (m_ActiveScene->m_SceneState)
			{
			case SceneState::Stop:
			{
				if (m_ViewportFocused)
					m_EditorCamera.OnUpdate(timestep);
				m_ActiveScene->OnEditorUpdate(timestep, m_EditorCamera);
				break;
			}
			case SceneState::Run:
			{
				m_ActiveScene->OnRuntimeUpdate(timestep);
				break;
			}
			case SceneState::Simulate:
			{
				m_ActiveScene->OnSimulationUpdate(timestep, m_EditorCamera);
				break;
			}
			case SceneState::Pause:
			{
				m_ActiveScene->OnPauseUpdate(timestep, m_EditorCamera);
				break;
			}
			default:
			{
				GE_ASSERT(false, "Unsupported Scene State. Active Scene will not Update.");
				break;
			}
			}

			// Handles Hovered Entity
			{
				auto [mx, my] = ImGui::GetMousePos();
				mx -= m_ViewportBounds[0].x;
				my -= m_ViewportBounds[0].y;
				glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
				my = viewportSize.y - my;

				int mouseX = (int)mx;
				int mouseY = (int)my;

				if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
				{
					int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY); // attachmentIndex = 1 - RED_INTEGER
					m_HoveredEntity = (pixelData == -1) ? Entity() : Entity{ (entt::entity)pixelData, m_ActiveScene.get() };
				}
			}
		}

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(GE_BIND_EVENT_FN(EditorLayer::OnMousePressed));
	}

	void EditorLayer::OnImGuiRender()
	{
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
					if (ImGui::MenuItem("Save Project", "Ctrl+Shift+S")) SaveProject();
					if (ImGui::MenuItem("Load Project", "Ctrl+Shift+O")) LoadProjectFromFile();

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Scene"))
				{
					if (ImGui::MenuItem("Save Scene", "Ctrl+S")) SaveScene(Project::GetActive()->GetSpec().SceneHandle);
					if (ImGui::MenuItem("Load Scene", "Ctrl+O")) LoadSceneFromFile();

					ImGui::EndMenu();
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) Application::GetApplication().Close();
				ImGui::EndMenu();	
			}

			if (ImGui::BeginMenu("Scripting"))
			{
				if (ImGui::MenuItem("Reload", "Ctrl+R")) Scripting::ReloadAssembly();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Renderer Statistics
		{
			ImGui::Begin("Renderer Statistics");

			Renderer2D::Statistics stats = Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats: ");
			ImGui::Text("Draw Calls - %d", stats.DrawCalls);
			ImGui::Text("Quad Count - %d", stats.SpawnCount);
			ImGui::Text("Vertices - %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices - %d", stats.GetTotalIndexCount());

			ImGui::Separator();

			{
				std::string name = "None";
				if (m_HoveredEntity != Entity{} && m_HoveredEntity.HasComponent<TagComponent>())
					name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
				ImGui::Text("Hovered Entity - %s", name.c_str());
			}

			ImGui::Separator();

			{
				ImGui::DragInt("Step Rate", &m_StepFrameMultiplier);
				ImGui::Checkbox("Camera Toggle", &m_UseEditorCamera);
			}

			ImGui::Separator();

			{
				if (s_font->GetTexture() != nullptr)
					ImGui::Image((ImTextureID)s_font->GetTexture()->GetID(), { 512, 512 }, { 0, 1 }, { 1, 0 });
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

		//	Viewport
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin("Viewport");

			auto viewportOffset = ImGui::GetWindowPos();
			auto viewportMin = ImGui::GetWindowContentRegionMin();
			auto viewportMax = ImGui::GetWindowContentRegionMax();
			m_ViewportBounds[0] = { viewportMin.x + viewportOffset.x, viewportMin.y + viewportOffset.y };
			m_ViewportBounds[1] = { viewportMax.x + viewportOffset.x, viewportMax.y + viewportOffset.y };

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();
			Application::GetApplication().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

			ImVec2 viewportSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportSize.x, viewportSize.y };

			uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
			ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PANEL_ITEM"))
				{
					const UUID handle = *(UUID*)payload->Data;

					if (Project::GetAssetManager<EditorAssetManager>()->HandleExists(handle))
					{
						LoadScene(handle);
					}
					else
					{
						GE_WARN("Asset Type is not Scene.");
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::End();
			ImGui::PopStyleVar();

		}

		UI_Toolbar();

		ImGui::End();

	}

	void EditorLayer::OnWindowResize(WindowResizeEvent& e)
	{
		RenderCommand::SetViewport((uint32_t)m_EditorCamera.GetPosition().x, (uint32_t)m_EditorCamera.GetPosition().y, e.GetWidth(), e.GetHeight());
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(KEY_LEFT_CONTROL) || Input::IsKeyPressed(KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(KEY_LEFT_SHIFT) || Input::IsKeyPressed(KEY_RIGHT_SHIFT);

		switch (e.GetKeyCode())
		{
		case KEY_D:
		{
			if (control)
				OnDuplicateEntity();
			break;
		}
		case KEY_N:
		{
			if (control)
			{
				if (shift)
					NewProject();

			}
			break;
		}
		case KEY_O:
		{
			if (control)
			{
				if (shift)
					LoadProjectFromFile();
				else
					LoadSceneFromFile();
			}
			break;
		}
		case KEY_S:
		{
			if (control)
			{
				if (shift)
					SaveProjectFromFile();
				else
					SaveProject();
			}

			break;
		}
		case KEY_R:
		{
			if (control)
			{
				Scripting::ReloadAssembly();
			}

			break;
		}
		case KEY_DELETE:
		{
			if (Application::GetApplication().GetImGuiLayer()->GetActiveWidgetID() == 0)
			{
				Entity selectedEntity = m_ScenePanel->GetSelectedEntity();
				if (selectedEntity)
				{
					m_ScenePanel->SetSelectedEntity(Entity{ });
					m_ActiveScene->DestroyEntity(selectedEntity);
				}
			}
			break;
		}
		default:
			GE_WARN("Key not bound.");
			break;
		}
		return true;
	}

	bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& e)
	{
		switch (e.GetMouseButton())
		{
		case GE_MOUSE_BUTTON_1:
			if (m_ViewportHovered && m_HoveredEntity)
				m_ScenePanel->SetSelectedEntity(m_HoveredEntity);
			break;
		case GE_MOUSE_BUTTON_2:
			break;
		default:
			GE_WARN("Mouse Button not bound.");
			break;
		}
		return true;
	}

	void EditorLayer::OnDuplicateEntity()
	{
		Entity selectedEntity = m_ScenePanel->GetSelectedEntity();
		if (m_ActiveScene->m_SceneState == SceneState::Stop && selectedEntity)
		{
			m_ActiveScene->DuplicateEntity(selectedEntity);
		}
	}

#pragma region Scene Functions

	void EditorLayer::OnSceneRuntime()
	{
		m_LastSceneState = m_ActiveScene->m_SceneState;
		if (m_LastSceneState != SceneState::Pause)
			m_EditorScene = Scene::Copy(m_ActiveScene); // Copy ActiveScene to revert after Run
		
		m_ActiveScene->OnRuntimeStart();

		m_ScenePanel->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnSceneSimulate()
	{
		m_LastSceneState = m_ActiveScene->m_SceneState;
		if (m_LastSceneState != SceneState::Pause)
			m_EditorScene = Scene::Copy(m_ActiveScene); // Copy ActiveScene to revert after Simulate

		m_ActiveScene->OnSimulationStart();

		m_ScenePanel->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnScenePause()
	{
		m_LastSceneState = m_ActiveScene->m_SceneState;
		m_ActiveScene->OnPauseStart();

		m_ScenePanel->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		m_ActiveScene->OnStop();
		m_ActiveScene = Scene::Copy(m_EditorScene); // Revert to copy of ActiveScene from Editor
		m_ScenePanel->SetScene(m_ActiveScene);
	}

	void EditorLayer::LoadSceneFromFile()
	{
		std::string filePath = FileDialogs::LoadFile("GAME Scene(*.scene)\0*.scene\0");
		if (!filePath.empty())
		{	
			Ref<Asset> asset = AssetImporter::ImportAsset(AssetMetadata(filePath));
			LoadScene(asset->GetHandle());
		}
	}

	void EditorLayer::LoadScene(UUID handle)
	{
		if (m_ActiveScene && m_ActiveScene->m_SceneState != SceneState::Stop)
			OnSceneStop();

		Project::GetActive()->GetSpec().SceneHandle = handle;
		GE_TRACE("Setting Active Scene from Asset Manager.");
		m_ActiveScene = Project::GetAsset<Scene>(handle);
		m_ActiveScene->OnResizeViewport((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_ScenePanel = CreateRef<SceneHierarchyPanel>(m_ActiveScene);
	}

	bool EditorLayer::SaveSceneFromFile()
	{
		std::string filePath = FileDialogs::SaveFile("GAME Scene(*.scene)\0 * .scene\0");
		if (!filePath.empty())
		{
			return AssetImporter::ExportAsset(m_ActiveScene->GetHandle(), AssetMetadata(filePath));
		}
		return false;
	}

	bool EditorLayer::SaveScene(UUID handle)
	{
		if(handle == 0)
			return SaveSceneFromFile();

		return Project::GetActive()->GetAssetManager()->SaveAsset(handle);
	}

#pragma endregion

#pragma region Project Functions

	bool EditorLayer::LoadProjectFromFile()
	{
		std::string filePath = FileDialogs::LoadFile("GAME Project(*.gproj)\0*.gproj\0");
		if (filePath.empty())
			return false;

		LoadProject(filePath);
		return true;
	}

	void EditorLayer::LoadProject(const std::filesystem::path& path)
	{
		if (path.extension().string() != ".gproj")
		{
			GE_WARN("Could not load {0} : File extension is not .gproj", path.filename().string());
			return;
		}
		GE_TRACE("Loading Project");
		if (Project::Load(path))
		{
			Project::NewAssetManager<EditorAssetManager>();
			Project::GetAssetManager<EditorAssetManager>()->DeserializeAssets();

			Scripting::Init();

			UUID handle = Project::GetActive()->GetSpec().SceneHandle;
			if (handle)
				LoadScene(handle);

			m_AssetPanel = CreateRef<AssetPanel>();

			GE_INFO("Project Load Complete");
		}
	}

	void EditorLayer::SaveProjectFromFile()
	{
		std::string filePath = FileDialogs::SaveFile("GAME Project(*.gproj)\0 * .gproj\0");
		if (!filePath.empty())
		{
			if (Project::Save(filePath))
			{
				UUID handle = Project::GetActive()->GetSpec().SceneHandle;
				if (handle)
					SaveScene(handle);

			}
		}
	}

	void EditorLayer::SaveProject()
	{
		if (!Project::GetProjectPath().empty())
		{
			std::filesystem::path projectPath = Project::GetProjectPath() / std::filesystem::path(Project::GetActive()->GetSpec().Name + ".gproj");
			if (Project::Save(projectPath))
			{
				UUID handle = Project::GetActive()->GetSpec().SceneHandle;
				if (handle)
					SaveScene(handle);
			}
		}
		else
			SaveProjectFromFile();
	}

	Ref<Project> EditorLayer::NewProject()
	{
		GE_WARN("Cannot make New Project.");
		return nullptr;
	}

#pragma endregion

#pragma region UI Panel Functions

	void EditorLayer::UI_Toolbar()
	{
		ImGui::Begin("##UItoolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		// Scene State Control Buttons
		if(m_ActiveScene)
		{

			if (!m_ActiveScene->IsPaused())
			{
				if (m_ActiveScene->m_SceneState != SceneState::Simulate)
				{
					// Scene Runtime Start & Stop
					Ref<Texture2D> playStopButtonTexture = m_ActiveScene->m_SceneState == SceneState::Run ? m_StopButtonTexture : m_PlayButtonTexture;

					ImGui::SameLine();
					if (ImGui::ImageButton(ImTextureID(playStopButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
					{
						(m_ActiveScene->m_SceneState == SceneState::Run) ? OnSceneStop() : OnSceneRuntime();
					}
				}

				// Scene Simulate Start & Stop
				if (!m_ActiveScene->IsRunning())
				{
					Ref<Texture2D> simulateStopButtonTexture = m_ActiveScene->m_SceneState == SceneState::Simulate ? m_StopButtonTexture : m_SimulateButtonTexture;
					
					ImGui::SameLine();
					if (ImGui::ImageButton(ImTextureID(simulateStopButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
					{
						(m_ActiveScene->m_SceneState == SceneState::Simulate) ? OnSceneStop() : OnSceneSimulate();
					}
				}
			}
			
			// Scene Pause during Runtime || Simulate
			if (m_ActiveScene->m_SceneState != SceneState::Stop)
			{
				Ref<Texture2D> playSimulatePauseButtonTexture = (m_ActiveScene->m_SceneState == SceneState::Run || m_ActiveScene->m_SceneState == SceneState::Simulate) ? m_PauseButtonTexture 
					: ( m_LastSceneState == SceneState::Simulate ? m_SimulateButtonTexture : (m_LastSceneState == SceneState::Run ? m_PlayButtonTexture : m_PauseButtonTexture) );
				ImGui::SameLine();
				if (ImGui::ImageButton(ImTextureID(playSimulatePauseButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
				{
					(m_ActiveScene->m_SceneState == SceneState::Run || m_ActiveScene->m_SceneState == SceneState::Simulate) ? OnScenePause() : (
						(m_ActiveScene->m_SceneState == SceneState::Pause && m_LastSceneState == SceneState::Simulate) ? OnSceneSimulate() : (
							(m_ActiveScene->m_SceneState == SceneState::Pause && m_LastSceneState == SceneState::Run) ? OnSceneRuntime() : GE_INFO("Could not execute Pause/Run/Simulate.")));
				}

				if (m_ActiveScene->IsPaused())
				{
					Ref<Texture2D> stepButtonTexture = m_StepButtonTexture;
					ImGui::SameLine();
					if (ImGui::ImageButton(ImTextureID(stepButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
						m_ActiveScene->OnStep(m_StepFrameMultiplier); // Adds step frames to queue. Handled in OnUpdate.

				}

			}
		}

		ImGui::End();
		
	}
#pragma endregion

}