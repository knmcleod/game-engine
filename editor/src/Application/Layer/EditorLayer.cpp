#include "EditorLayer.h"

#include "../../AssetManager/EditorAssetManager.h"

#include <GE/Core/Application/Application.h>
#include <GE/Core/Input/Input.h>
#include <GE/Core/FileSystem/FileSystem.h>

#include <GE/Project/Project.h>
#include <GE/Rendering/RenderCommand.h>
#include <GE/Scripting/Scripting.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace GE
{
	static void InitializeRuntimeAssetManager()
	{
		AssetMap assetMap = Project::GetAssetManager<EditorAssetManager>()->GetLoadedAssets();
		Ref<RuntimeAssetManager> ram = Project::NewAssetManager<RuntimeAssetManager>();
		for (const auto& [uuid, asset] : assetMap)
		{
			ram->AddAsset(asset);
		}

		if (ram->SerializeAssets())
			GE_INFO("InitializeRuntimeAssetManager Complete");

		Project::NewAssetManager<EditorAssetManager>()->DeserializeAssets();
	}

	EditorLayer::EditorLayer(const std::string& name)
		: Layer(name), m_ViewportBounds{ { glm::vec2() },{ glm::vec2() } }
	{

	}

	void EditorLayer::OnAttach()
	{
		GE_PROFILE_FUNCTION();
		GE_INFO("EditorLayer::OnAttach Start.");
		{
			GE_PROFILE_SCOPE("EditorLayer::OnAttach - Framebuffer Setup");
			Framebuffer::Config framebufferConfig;
			framebufferConfig.AttachmentSpecification = { Framebuffer::TextureFormat::RGBA8, 
				Framebuffer::TextureFormat::RED_INTEGER, Framebuffer::TextureFormat::DEPTH24STENCIL8 };
			framebufferConfig.Width = Application::GetApp().GetWidth();
			framebufferConfig.Height = Application::GetApp().GetHeight();
			m_Framebuffer = Framebuffer::Create(framebufferConfig);
		}
		m_EditorCamera = new EditorCamera(45.0f, 0.1f, 100.0f, (float)m_Framebuffer->GetWidth()/(float)m_Framebuffer->GetHeight());

		m_PlayButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Play_Button.png")->GetHandle();
		m_SimulateButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Simulate_Button.png")->GetHandle();
		m_PauseButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Pause_Button.png")->GetHandle();
		m_StepButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Step_Button.png")->GetHandle();
		m_StopButtonHandle = Project::GetAssetManager<EditorAssetManager>()->GetAsset("textures/Stop_Button.png")->GetHandle();

		LoadScene(Project::GetSceneHandle());

		GE_INFO("EditorLayer::OnAttach Complete.");
	}

	void EditorLayer::OnDetach()
	{
		GE_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
		RenderCommand::ClearAPI();

		// Clear attachmentIndex = 1 - RED_INTEGER / entityID
		m_Framebuffer->ClearAttachment(1, -1);

		if (m_RuntimeScene)
		{
			UpdateScene(ts);

			// if Stopped, Simulating, or Paused
			if(!m_RuntimeScene->IsRunning())
			{
				auto [mx, my] = ImGui::GetMousePos();
				mx -= m_ViewportBounds[0].x;
				my -= m_ViewportBounds[0].y;
				glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
				my = viewportSize.y - my;

				int mouseX = mx;
				int mouseY = my;

				if (mouseX >= 0 && mouseY >= 0 && mouseX < viewportSize.x && mouseY < viewportSize.y)
				{
					int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY); // attachmentIndex = 1 - RED_INTEGER
					m_HoveredEntity = (pixelData == -1) ? Entity() : Entity((entt::entity)pixelData, m_RuntimeScene.get());
				}

				const Camera& currentCamera = (m_UseEditorCamera ? *m_EditorCamera : (Camera&)(m_RuntimeScene->GetPrimaryCameraEntity().GetComponent<CameraComponent>().ActiveCamera));

				if (m_HoveredEntity != Entity() && m_HoveredEntity.HasComponent<TransformComponent>())
				{
					Renderer2D::Start(currentCamera);

					TransformComponent tc = m_HoveredEntity.GetComponent<TransformComponent>();

					// Outline
					Renderer2D::DrawRectangle(tc.GetTransform(), m_HoveredColor, m_HoveredEntity.GetEntityID());
					
					Renderer2D::End();
				}

				const Entity& selectedEntity = m_ScenePanel->GetSelectedEntity();
				if (selectedEntity != Entity()  && selectedEntity.HasComponent<TransformComponent>())
				{
					Renderer2D::Start(currentCamera);

					TransformComponent tc = selectedEntity.GetComponent<TransformComponent>();

					// Outline
					Renderer2D::DrawRectangle(tc.GetTransform(), m_ScenePanel->GetSelectedColor(), selectedEntity.GetEntityID());
					
					Renderer2D::End();
				}
			}

		}

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera->OnEvent(e);

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
					if (ImGui::MenuItem("Save Project", "Ctrl+Shift+S")) Application::SaveAppProject();
					if (ImGui::MenuItem("Load Project", "Ctrl+Shift+O")) Application::LoadAppProjectFileDialog();
					if(ImGui::MenuItem("Export", "Ctrl+Shift+E")) InitializeRuntimeAssetManager();
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
				Renderer2D::Statistics stats = Renderer2D::GetStats();
				ImGui::Text("Renderer2D");
				ImGui::Text("\tDraw Calls - %d", stats.DrawCalls);
				ImGui::Text("\tSpawn Count - %d", stats.SpawnCount);
				ImGui::Text("\tVertices - %d", stats.GetTotalVertexCount());
				ImGui::Text("\tIndices - %d", stats.GetTotalIndexCount());
			}

			ImGui::Separator();

			{
				ImGui::Text("Scene");
				ImGui::DragInt("Step Rate", &m_StepFrameMultiplier);
				ImGui::Checkbox("Editor Camera Toggle", &m_UseEditorCamera);

				std::string name = "None";
				if (m_HoveredEntity != Entity() && m_HoveredEntity.HasComponent<TagComponent>())
					name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
				ImGui::Text("Hovered Entity - %s", name.c_str());
				ImGui::DragFloat4("Hovered Color", glm::value_ptr(m_HoveredColor));

				ImGui::DragFloat4("Selected Color", glm::value_ptr(m_ScenePanel->m_SelectedColor));
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

			Application::BlockAppEvents(!m_ViewportFocused && !m_ViewportHovered);
			
			ImVec2 viewportSize = ImGui::GetContentRegionAvail();			
			m_Framebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

			uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
			ImGui::Image((ImTextureID)textureID, viewportSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
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
		RenderCommand::SetViewport((uint32_t)m_EditorCamera->GetPosition().x, (uint32_t)m_EditorCamera->GetPosition().y, e.GetWidth(), e.GetHeight());
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
				OnDuplicateEntity();
			break;
		}
		case Input::KEY_E:
		{
			if(control)
				if(shift)
					InitializeRuntimeAssetManager();
		}
		case Input::KEY_F:
		{
			if(control)
				Application::GetApp().GetWindow().SetFullscreen(true);
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
			if (Application::GetActiveWidgetID() == 0)
			{
				Entity selectedEntity = m_ScenePanel->GetSelectedEntity();
				if (selectedEntity)
				{
					m_ScenePanel->m_SelectedEntity = Entity();
					m_RuntimeScene->DestroyEntity(selectedEntity);
				}
			}
			break;
		}
		case Input::KEY_ESCAPE:
		{
			Application::GetApp().GetWindow().SetFullscreen(false);
			break;
		}
		default:
			GE_WARN("Editor::OnKeyPressed Event - Input::Key not bound. Keycode : {0}", e.GetKeyCode());
			break;
		}
		return true;
	}

	bool EditorLayer::OnMousePressed(MouseButtonPressedEvent& e)
	{
		switch (e.GetMouseButton())
		{
		case Input::MOUSE_BUTTON_1:
			if (m_ViewportHovered && m_HoveredEntity)
				m_ScenePanel->m_SelectedEntity = m_HoveredEntity;
			break;
		case Input::MOUSE_BUTTON_2:
			break;
		default:
			GE_WARN("Editor Input::ButtonPressed not bound.");
			break;
		}
		return true;
	}

	void EditorLayer::OnDuplicateEntity()
	{
		Entity selectedEntity = m_ScenePanel->GetSelectedEntity();
		if (m_RuntimeScene->GetState() == Scene::State::Stop && selectedEntity)
		{
			m_RuntimeScene->DuplicateEntity(selectedEntity);
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
		if (state != Scene::State::Pause && !m_RuntimeScene->IsPaused())
		{
			m_RuntimeScene = Project::GetCopy<Scene>(m_EditorScene);
			m_ScenePanel->SetScene(m_RuntimeScene.get());
			m_HoveredEntity = Entity();
		}
		if (state == Scene::State::Run)
			m_UseEditorCamera = false;

		m_RuntimeScene->OnStart(state, m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight());
	}

	void EditorLayer::UpdateScene(Timestep ts)
	{
		if ((m_RuntimeScene->IsStopped() || m_UseEditorCamera) && m_EditorCamera)
		{
			m_EditorCamera->SetViewport(m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight());

			m_RuntimeScene->OnUpdate(ts, m_EditorCamera);
		}
		else
		{
			m_RuntimeScene->OnUpdate(ts);
		}
	}

	void EditorLayer::StopScene()
	{
		if (m_RuntimeScene)
		{
			m_RuntimeScene->OnStop();
		}
		m_UseEditorCamera = true;

		// Old SelectedEntity.UUID won't exist after setting runtimeScene=editorScene
		// Save a copy to revert to after
		UUID selectedUUID = 0;
		if (m_ScenePanel->GetSelectedEntity() && m_ScenePanel->GetSelectedEntity().HasComponent<IDComponent>())
		{
			selectedUUID = m_ScenePanel->GetSelectedEntity().GetComponent<IDComponent>().ID;
			m_ScenePanel->m_SelectedEntity = Entity();
		}
		m_RuntimeScene = m_EditorScene;
		m_ScenePanel->SetScene(m_RuntimeScene.get(), selectedUUID);
	}

	void EditorLayer::LoadScene(UUID handle)
	{
		if (m_RuntimeScene)
		{
			if (m_RuntimeScene->GetHandle() == handle)
				return;

			if (!m_RuntimeScene->IsStopped())
				m_RuntimeScene->OnStop();
		}

		m_EditorScene = Project::GetAsset<Scene>(handle);
		m_RuntimeScene = m_EditorScene;
		if (m_RuntimeScene)
		{
			m_AssetPanel = CreateRef<AssetPanel>();
			m_ScenePanel = CreateRef<SceneHierarchyPanel>(m_RuntimeScene.get());
			GE_TRACE("Editor Loaded Scene\n\tName: {0}", m_RuntimeScene->GetName());
		}
	}

#pragma endregion

#pragma region UI Panel Functions
	void EditorLayer::UI_Toolbar()
	{
		ImGui::Begin("##UItoolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		// Scene State Control Buttons
		if(m_RuntimeScene)
		{
			// Stop <-> Run || Simulate
			if (!m_RuntimeScene->IsPaused())
			{
				const Scene::State& currentState = GetSceneState();

				// Scene Runtime Start & Stop
				if (currentState != Scene::State::Simulate)
				{
					Scene::State handledState = Scene::State::Run;
					Ref<Texture2D> playStopButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_StopButtonHandle : m_PlayButtonHandle);

					ImGui::SameLine();
					if (ImGui::ImageButton((ImTextureID)playStopButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
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
					if (ImGui::ImageButton((ImTextureID)simulateStopButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
					{
						(currentState == handledState) ? StopScene() : StartScene(handledState);
					}
				}
			}
			
			// Pause <-> Run || Simulate
			if (!m_RuntimeScene->IsStopped())
			{
				const Scene::State& currentState = GetSceneState();

				// Simulate
				if (currentState != Scene::State::Run)
				{
					Scene::State handledState = Scene::State::Simulate;

					Ref<Texture2D> simulatePauseButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_PauseButtonHandle : m_SimulateButtonHandle);

					ImGui::SameLine();
					if (ImGui::ImageButton((ImTextureID)simulatePauseButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
					{
						(currentState == handledState) ? StartScene(Scene::State::Pause) : StartScene(handledState);
					}
				}
				// Run
				else if (currentState != Scene::State::Simulate)
				{
					Scene::State handledState = Scene::State::Run;

					Ref<Texture2D> playPauseButtonTexture = Project::GetAsset<Texture2D>(currentState == handledState ? m_PauseButtonHandle : m_PlayButtonHandle);

					ImGui::SameLine();
					if (ImGui::ImageButton((ImTextureID)playPauseButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
					{
						(currentState == handledState) ? StartScene(Scene::State::Pause) : StartScene(handledState);
					}
				}

				// Step Pause
				if (m_RuntimeScene->IsPaused())
				{
					Ref<Texture2D> stepButtonTexture = Project::GetAsset<Texture2D>(m_StepButtonHandle);
					ImGui::SameLine();
					if (ImGui::ImageButton((ImTextureID)stepButtonTexture->GetID(), ImVec2(20.0f, 20.0f)))
						m_RuntimeScene->OnStep(m_StepFrameMultiplier); // Adds step frames to queue. Handled in Scene::OnPauseUpdate.
				}

			}
		}

		ImGui::End();
		
	}
#pragma endregion

}