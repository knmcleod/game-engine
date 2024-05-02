#include "EditorLayer.h"

#include "../AssetManager/EditorAssetManager.h"

#include <GE/GE.h>

namespace GE
{
	EditorLayer::EditorLayer(const std::string& name)
		: Layer(name), m_ViewportSize(1.0f), m_ViewportBounds{ { glm::vec2() },{ glm::vec2() } }
	{
	}

	void EditorLayer::OnAttach()
	{
		GE_PROFILE_FUNCTION();
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
			if(!LoadProject(commandLineArgs[1]))
				Application::GetApplication().Close();
		}
		else
		{
			if (!LoadProjectFromFile())
				Application::GetApplication().Close();
		}

		m_PlayButtonHandle = Project::GetAsset<Texture2D>("textures/Play_Button.png")->GetHandle();
		m_SimulateButtonHandle = Project::GetAsset<Texture2D>("textures/Simulate_Button.png")->GetHandle();
		m_PauseButtonHandle = Project::GetAsset<Texture2D>("textures/Pause_Button.png")->GetHandle();
		m_StepButtonHandle = Project::GetAsset<Texture2D>("textures/Step_Button.png")->GetHandle();
		m_StopButtonHandle = Project::GetAsset<Texture2D>("textures/Stop_Button.png")->GetHandle();

		GE_INFO("Editor Layer OnAttach Complete.");
	}

	void EditorLayer::OnDetach()
	{
		GE_PROFILE_FUNCTION();

		Scripting::Shutdown();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
		RenderCommand::ClearAPI();

		// Clear attachmentIndex = 1 - RED_INTEGER / entityID
		m_Framebuffer->ClearAttachment(1, -1);

		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if (m_ActiveScene)
		{
			if (m_UseEditorCamera && &m_EditorCamera)
			{
				UpdateScene(ts, &m_EditorCamera);
			}
			else
			{
				UpdateScene(ts);
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
					m_HoveredEntity = (pixelData == -1) ? Entity(entt::null, m_ActiveScene.get()) : Entity((entt::entity)pixelData, m_ActiveScene.get());
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

		// Statistics
		{
			ImGui::Begin("Statistics");

			{
				Renderer2D::Statistics stats = Renderer2D::GetStats();
				ImGui::Text("Renderer2D Stats");
				ImGui::Text("\tDraw Calls - %d", stats.DrawCalls);
				ImGui::Text("\tQuad Count - %d", stats.SpawnCount);
				ImGui::Text("\tVertices - %d", stats.GetTotalVertexCount());
				ImGui::Text("\tIndices - %d", stats.GetTotalIndexCount());
			}

			ImGui::Separator();

			{
				ImGui::Text("Mouse Picking Stats");
				std::string name = "None";
				if (m_HoveredEntity != Entity() && m_HoveredEntity.HasComponent<TagComponent>())
					name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
				ImGui::Text("Hovered Entity - %s", name.c_str());
			}

			ImGui::Separator();

			{
				ImGui::Text("Scene Stats");
				ImGui::DragInt("Step Rate", &m_StepFrameMultiplier);
				ImGui::Checkbox("Camera Toggle", &m_UseEditorCamera);
			}

			ImGui::Separator();

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

					Ref<Asset> asset = Project::GetAssetManager<EditorAssetManager>()->GetAsset(handle);
					Asset::Type type = asset->GetType();
					if (asset && type == Asset::Type::Scene)
					{
						LoadScene(handle);
					}
					else
					{
						GE_WARN("Asset Type is not Scene.\n\tType:{0}", AssetUtils::AssetTypeToString(type));
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
					m_ScenePanel->SetSelectedEntity(Entity());
					m_ActiveScene->DestroyEntity(selectedEntity);
				}
			}
			break;
		}
		default:
			GE_WARN("Editor Layer Key not bound.");
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
			GE_WARN("Editor Layer Button not bound.\n\tKey:{0}", e.GetName());
			break;
		}
		return true;
	}

	void EditorLayer::OnDuplicateEntity()
	{
		Entity selectedEntity = m_ScenePanel->GetSelectedEntity();
		if (m_ActiveScene->GetState() == Scene::State::Stop && selectedEntity)
		{
			m_ActiveScene->DuplicateEntity(selectedEntity);
		}
	}

#pragma region Scene Functions

	void EditorLayer::StartScene(const Scene::State& state)
	{
		if (state == Scene::State::Stop)
		{
			StopScene();
			return;
		}

		m_ActiveScene->OnStart(state, m_ViewportSize.x, m_ViewportSize.y);
	}

	void EditorLayer::UpdateScene(Timestep ts, Camera* camera)
	{
		m_ActiveScene->OnUpdate(ts, camera);
	}

	void EditorLayer::StopScene()
	{
		// Stop active processes & revert to stopped/saved scene asset

		if (m_ActiveScene)
		{
			m_ActiveScene->OnStop();
		}

		UUID sceneHandle = Project::GetConfig().SceneHandle;
		m_ActiveScene = Project::GetAsset<Scene>(sceneHandle)->Copy();
		m_ScenePanel->SetScene(m_ActiveScene);
	}

	void EditorLayer::LoadSceneFromFile()
	{
		std::string filePath = FileDialogs::LoadFile("GAME Scene(*.scene)\0*.scene\0");
		if (!filePath.empty())
		{
			Ref<Scene> scene = Project::GetAsset<Scene>(filePath);
			if (scene)
				LoadScene(scene->GetHandle());
			else
			{
				AssetMetadata metadata(UUID(), filePath);
				Project::GetAssetManager<EditorAssetManager>()->AddAsset(metadata);
			}
		}
	}

	void EditorLayer::LoadScene(UUID handle)
	{
		if (m_ActiveScene)
		{
			if (m_ActiveScene->GetHandle() == handle)
				return;

			if (!m_ActiveScene->IsStopped())
				m_ActiveScene->OnStop();
		}

		m_ActiveScene = Project::GetAsset<Scene>(handle)->Copy();
		if (m_ActiveScene)
		{
			m_ScenePanel->SetScene(m_ActiveScene);
			GE_TRACE("Editor Loaded Scene\n\tName: {0}", m_ActiveScene->GetName());
		}
	}

#pragma endregion

#pragma region Project Functions

	bool EditorLayer::LoadProjectFromFile()
	{
		std::string filePath = FileDialogs::LoadFile("GAME Project(*.gproj)\0*.gproj\0");
		if (filePath.empty())
			return false;

		return LoadProject(filePath);
	}

	bool EditorLayer::LoadProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{
			Scripting::Init();

			if(Project::NewAssetManager<EditorAssetManager>()->DeserializeAssets())
			{
				m_AssetPanel = CreateRef<AssetPanel>();
				m_ActiveScene = CreateRef<Scene>();
				m_ScenePanel = CreateRef<SceneHierarchyPanel>(m_ActiveScene);

				UUID handle = Project::GetConfig().SceneHandle;
				if (handle)
					LoadScene(handle);

				GE_INFO("Editor Load Project Complete");
				return true;
			}

		}

		GE_WARN("Editor Load Project Failed");
		return false;
	}

	void EditorLayer::SaveProjectFromFile()
	{
		std::string filePath = FileDialogs::SaveFile("GAME Project(*.gproj)\0 * .gproj\0");
		if (filePath.empty())
			return;

		SaveProject(filePath);
	}

	void EditorLayer::SaveProject(const std::filesystem::path& path)
	{
		path.extension() = ".gproj";
		if (Project::Save(path))
		{
			Project::GetAssetManager<EditorAssetManager>()->SerializeAssets();
			GE_INFO("Editor Save Project Complete");
		}
	}

	void EditorLayer::SaveProject()
	{
		const std::filesystem::path& path = Project::GetProjectPath() / std::filesystem::path(Project::GetConfig().Name + ".gproj");
		if (!path.empty())
		{
			SaveProject(path);
		}
		else
			SaveProjectFromFile();
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
				const Scene::State& currentState = GetSceneState();

				// Scene Runtime Start & Stop
				if (currentState != Scene::State::Simulate)
				{
					Scene::State handledState = Scene::State::Run;
					Ref<Texture2D> playStopButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_StopButtonHandle : m_PlayButtonHandle);

					ImGui::SameLine();
					if (ImGui::ImageButton(ImTextureID(playStopButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
					{
						(currentState == handledState) ? StopScene() : StartScene(handledState);
					}
				}

				// Scene Simulate Start & Stop
				if (currentState != Scene::State::Run)
				{
					Scene::State handledState = Scene::State::Simulate;
					Ref<Texture2D> simulateStopButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_StopButtonHandle : m_SimulateButtonHandle);
					
					ImGui::SameLine();
					if (ImGui::ImageButton(ImTextureID(simulateStopButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
					{
						(currentState == handledState) ? StopScene() : StartScene(handledState);
					}
				}
			}
			
			// Scene Pause during Runtime || Simulate
			if (!m_ActiveScene->IsStopped())
			{
				const Scene::State& currentState = GetSceneState();

				// Simulate
				if (currentState != Scene::State::Run)
				{
					Scene::State handledState = Scene::State::Simulate;

					Ref<Texture2D> simulatePauseButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_PauseButtonHandle : m_SimulateButtonHandle);

					ImGui::SameLine();
					if (ImGui::ImageButton(ImTextureID(simulatePauseButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
					{
						(currentState == handledState) ? StartScene(Scene::State::Pause) : StartScene(handledState);
					}
				}

				// Run
				if (currentState != Scene::State::Simulate)
				{
					Scene::State handledState = Scene::State::Run;

					Ref<Texture2D> playPauseButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_PauseButtonHandle : m_PlayButtonHandle);

					ImGui::SameLine();
					if (ImGui::ImageButton(ImTextureID(playPauseButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
					{
						(currentState == handledState) ? StartScene(Scene::State::Pause) : StartScene(handledState);
					}
				}

				// Step Pause
				if (m_ActiveScene->IsPaused())
				{
					Ref<Texture2D> stepButtonTexture = Project::GetAsset<Texture2D>(m_StepButtonHandle);
					ImGui::SameLine();
					if (ImGui::ImageButton(ImTextureID(stepButtonTexture->GetID()), ImVec2(20.0f, 20.0f)))
						m_ActiveScene->OnStep(m_StepFrameMultiplier); // Adds step frames to queue. Handled in OnPauseUpdate.
				}

			}
		}

		ImGui::End();
		
	}
#pragma endregion

}