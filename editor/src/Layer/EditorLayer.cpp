#include "EditorLayer.h"

//#define ENTITYTEST

namespace GE
{
	EditorLayer::EditorLayer(const std::string& name)
		: Layer(name)
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
		m_ActiveScene = CreateRef<Scene>();
		m_ScenePanel = CreateRef<SceneHierarchyPanel>(m_ActiveScene);

		m_EditorCamera = EditorCamera(45.0f, 1.778f, 0.1f, 100.0f);

		#if ENTITYTEST
			m_CameraEntityPrimary = m_ActiveScene->CreateEntity("Primary Camera Entity");
			m_CameraEntityPrimary.AddComponent<CameraComponent>();
			m_CameraEntityPrimary.GetComponent<CameraComponent>().Primary = true;
			m_CameraEntityPrimary.AddComponent<NativeScriptComponent>().Bind<CameraController>();

			m_CameraEntitySecondary = m_ActiveScene->CreateEntity("Secondary Camera Entity");
			m_CameraEntitySecondary.AddComponent<CameraComponent>();
			m_CameraEntitySecondary.AddComponent<NativeScriptComponent>().Bind<CameraController>();

			m_SquareEntity = m_ActiveScene->CreateEntity();
			m_SquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		#endif
	}

	void EditorLayer::OnDetach()
	{
		GE_PROFILE_FUNCTION();
		RenderCommand::ShutDown();
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
			m_ActiveScene->ResizeViewport((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if(m_ViewportFocused)
			m_EditorCamera.OnUpdate(timestep);

		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
		RenderCommand::ClearAPI();

		// Clear attachmentIndex = 1 - RED_INTEGER / entityID
		m_Framebuffer->ClearAttachment(1, -1);

		m_ActiveScene->OnUpdateEditor(timestep, m_EditorCamera);

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
				m_Framebuffer->ReadPixel(1, mouseX, mouseY); // attachmentIndex = 1 - RED_INTEGER
				GE_CORE_INFO("Viewport Mouse - ({0}, {1})", mouseX, mouseY);
			}
		}

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(GE_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
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
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("New", "Ctrl+N")) NewScene();
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) SaveSceneAs();
				if (ImGui::MenuItem("Load", "Ctrl+O")) LoadScene();
				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) Application::GetApplication().Close();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		{
			ImGui::Begin("Statistics");

			Renderer2D::Statistics stats = Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats: ");
			ImGui::Text("Draw Calls - %d", stats.DrawCalls);
			ImGui::Text("Quad Count - %d", stats.QuadCount);
			ImGui::Text("Vertices - %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices - %d", stats.GetTotalIndexCount());

			ImGui::End();
		}

		{
			if (m_ScenePanel)
			{
				m_ScenePanel->OnImGuiRender();
			}
		}

		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin("Viewport");

			auto viewportOffset = ImGui::GetCursorPos();

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();
			Application::GetApplication().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);
	
			ImVec2 viewportSize = ImGui::GetContentRegionAvail();
			m_ViewportSize = { viewportSize.x, viewportSize.y };
	
			uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
			ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			auto windowSize = ImGui::GetWindowSize();
			ImVec2 minBound = ImGui::GetWindowPos();
			minBound.x += viewportOffset.x;
			minBound.y += viewportOffset.y;

			ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
			m_ViewportBounds[0] = { minBound.x, minBound.y };
			m_ViewportBounds[1] = { maxBound.x, maxBound.y };

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

		bool control = Input::IsKeyPressed(GE_KEY_LEFT_CONTROL) || Input::IsKeyPressed(GE_KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(GE_KEY_LEFT_SHIFT) || Input::IsKeyPressed(GE_KEY_RIGHT_SHIFT);

		switch (e.GetKeyCode())
		{
		// File
		case GE_KEY_N:
		{
			if (control)
				NewScene();
			break;
		}
		case GE_KEY_O:
		{
			if (control)
				LoadScene();
			break;
		}
		case GE_KEY_S:
		{
			if (control && shift)
				SaveSceneAs();
			break;
		}
		default:
			break;
		}
		return true;
	}

	void EditorLayer::LoadScene()
	{
		std::string filePath = FileDialogs::LoadFile("GE Scene (*.ge)\0*.ge\0");
		if (!filePath.empty())
		{
			NewScene();

			SceneSerializer serializer(m_ActiveScene);
			serializer.DeserializeText(filePath);
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filePath = FileDialogs::SaveFile("GE Scene (*.ge)\0*.ge\0");
		if (!filePath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.SerializeText(filePath);
		}
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->ResizeViewport((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_ScenePanel->SetScene(m_ActiveScene);
	}
}