#include "EditorLayer.h"
#include "GE/Scripting/Scripting.h"

namespace GE
{
	extern const std::filesystem::path g_AssetsPath;

	EditorLayer::EditorLayer(const std::string& name)
		: Layer(name), m_ViewportSize(1.0f)
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

		//Scene
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = Scene::Copy(m_EditorScene);

		m_ScenePanel = CreateRef<SceneHierarchyPanel>(m_ActiveScene);
		m_AssetPanel = CreateRef<AssetPanel>();

		m_EditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 100.0f);

		m_PlayButtonTexture = Texture2D::Create("assets/textures/UI/Play_Button.png");
		m_PauseButtonTexture = Texture2D::Create("assets/textures/UI/Pause_Button.png");

	}

	void EditorLayer::OnDetach()
	{
		GE_PROFILE_FUNCTION();
		RenderCommand::ShutDown();
	}

	void EditorLayer::OnUpdate(Timestep timestep)
	{
		// Resize
		m_ActiveScene->OnResizeViewport((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

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

		switch (m_ActiveScene->m_SceneState)
		{
		case Scene::SceneState::Stop:
		{
			if (m_ViewportFocused)
				m_EditorCamera.OnUpdate(timestep);
			m_ActiveScene->OnEditorUpdate(timestep, m_EditorCamera);
			break;
		}
		case Scene::SceneState::Run:
		{
			m_ActiveScene->OnRuntimeUpdate(timestep);
			break;
		}
		case Scene::SceneState::Simulate:
		{
			m_EditorCamera.OnUpdate(timestep);

			m_ActiveScene->OnSimulationUpdate(timestep, m_EditorCamera);
			break;
		}
		default:
		{
			GE_CORE_ASSERT(false, "Unsupported Scene State. Active Scene will not Update.");
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
				if (ImGui::MenuItem("New", "Ctrl+N")) NewScene();
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) SaveSceneFromFile();
				if (ImGui::MenuItem("Load", "Ctrl+O")) LoadSceneFromFile();
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

			Renderer2D::Statistics stats = Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats: ");
			ImGui::Text("Draw Calls - %d", stats.DrawCalls);
			ImGui::Text("Quad Count - %d", stats.SpawnCount);
			ImGui::Text("Vertices - %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices - %d", stats.GetTotalIndexCount());

			ImGui::Separator();

			std::string name = "None";
			if (m_HoveredEntity && m_HoveredEntity.HasComponent<TagComponent>())
				name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
			ImGui::Text("Hovered Entity - %s", name.c_str());

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
					const wchar_t* path = (const wchar_t*)payload->Data;

					LoadScene(std::filesystem::path(g_AssetsPath) / path);
				}
				ImGui::EndDragDropTarget();
			}

			UI_Toolbar();

			ImGui::End();
			ImGui::PopStyleVar();

		}

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
			// File
		case KEY_D:
		{
			if (control)
				OnDuplicateEntity();
			break;
		}
		case KEY_N:
		{
			if (control)
				NewScene();
			break;
		}
		case KEY_O:
		{
			if (control)
				LoadSceneFromFile();
			break;
		}
		case KEY_S:
		{
			if (control)
			{
				if(shift)
					SaveSceneFromFile();
				else
					SaveScene();
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
		default:
			GE_CORE_INFO("Key not bound.");
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
			GE_CORE_WARN("Mouse Button not bound.");
			break;
		}
		return true;
	}

	void EditorLayer::OnDuplicateEntity()
	{
		Entity selectedEntity = m_ScenePanel->GetSelectedEntity();
		if (m_ActiveScene->m_SceneState == Scene::SceneState::Stop && selectedEntity)
		{
			m_ActiveScene->DuplicateEntity(selectedEntity);
		}
	}

#pragma region Scene Functions

	void EditorLayer::OnSceneRuntime()
	{
		m_EditorScene = Scene::Copy(m_ActiveScene); // Copy ActiveScene(Editor) to revert after Run
		m_ActiveScene->OnRuntimeStart();

		m_ScenePanel->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnSceneSimulate()
	{
		m_EditorScene = Scene::Copy(m_ActiveScene); // Copy ActiveScene(Editor) to revert after Simulate
		m_ActiveScene->OnSimulationStart();

		m_ScenePanel->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		m_ActiveScene->OnStop();
		m_ActiveScene = Scene::Copy(m_EditorScene); // Revert to copy of ActiveScene(Editor)

		m_ScenePanel->SetScene(m_ActiveScene);
	}

	void EditorLayer::LoadSceneFromFile()
	{
		std::string filePath = FileDialogs::LoadFile("GAME Scene (*.game)\0*.game\0");
		if (!filePath.empty())
			LoadScene(filePath);
	}

	void EditorLayer::LoadScene(const std::filesystem::path& path)
	{
		if (path.extension().string() != ".game")
		{
			GE_CORE_WARN("Could not load {0} : File extension is not .game", path.filename().string());
			return;
		}

		if (m_ActiveScene && m_ActiveScene->m_SceneState != Scene::SceneState::Stop)
			OnSceneStop();

		m_ActiveScene = CreateRef<Scene>();
		SceneSerializer serializer(m_ActiveScene);
		if (serializer.DeserializeText(path.string()))
		{
			m_ActiveScene->OnResizeViewport((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_ActiveScene->SetName(path.filename().string());
			m_ScenePanel->SetScene(m_ActiveScene);
			m_ScenePath = path;
		}
	}

	void EditorLayer::SaveSceneFromFile()
	{
		std::string filePath = FileDialogs::SaveFile("GAME Scene(*.game)\0 * .game\0");
		if (!filePath.empty())
		{
			m_ScenePath = filePath;
			SerializeScene(m_ActiveScene, m_ScenePath);
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_ScenePath.empty() && m_ActiveScene)
			SerializeScene(m_ActiveScene, m_ScenePath);
		else
			SaveSceneFromFile();
	}

	void EditorLayer::NewScene()
	{
		if (m_ActiveScene && m_ActiveScene->m_SceneState != Scene::SceneState::Stop)
			OnSceneStop();

		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnResizeViewport((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_ScenePanel->SetScene(m_ActiveScene);
		m_ScenePath = std::filesystem::path();
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		if (path.filename().extension().string() != ".game")
		{
			GE_CORE_WARN("Could not save {0}	: File extension is not .game", path.filename().string());
			return;
		}

		scene->m_Name = path.filename().string();

		SceneSerializer serializer(scene);
		serializer.SerializeText(path.string());
	}

#pragma endregion

#pragma region UI Panel Functions

	void EditorLayer::UI_Toolbar()
	{
		ImGui::Begin("##UItoolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 5.0f;
		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size *0.5f));
		Ref<Texture2D> icon = m_ActiveScene->m_SceneState == Scene::SceneState::Run || m_ActiveScene->m_SceneState == Scene::SceneState::Simulate 
			? m_PauseButtonTexture : m_PlayButtonTexture;
		if (ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(10.0f, 10.0f)))
		{
			if (m_ActiveScene->m_SceneState == Scene::SceneState::Run)
				OnSceneStop();
			else if (m_ActiveScene->m_SceneState == Scene::SceneState::Stop)
				OnSceneRuntime();
		}
		/*if (ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(10.0f, 10.0f)))
		{
			if (m_ActiveScene->m_SceneState == Scene::SceneState::Simulate)
				OnSceneStop();
			else if (m_ActiveScene->m_SceneState == Scene::SceneState::Stop)
				OnSceneSimulate();
		}*/

		ImGui::End();
	}
#pragma endregion

}