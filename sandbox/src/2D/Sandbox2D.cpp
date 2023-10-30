#include "Sandbox2D.h"

Sandbox2D::Sandbox2D(const std::string& name) : Layer(name), m_OrthoCameraController(GE::Application::GetApplication().GetWindow().GetWidth(), GE::Application::GetApplication().GetWindow().GetHeight())
{
}

void Sandbox2D::OnAttach()
{
	GE::RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });

	m_SpriteSheet = GE::Texture2D::Create("assets/textures/Tilemap/inputSpriteSheet.png");

	m_Sprite = GE::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 1 }, { 16, 16 });
}

void Sandbox2D::OnDetach()
{
	GE::RenderCommand::ShutDown();
}

void Sandbox2D::OnUpdate(GE::Timestep timestep)
{
	m_OrthoCameraController.OnUpdate(timestep);

	GE::Renderer2D::ResetStats();
	GE::RenderCommand::ClearAPI();
	
	static float rotation = 0.0f;
	rotation += timestep * 20.0f;

	GE::Renderer2D::Start(m_OrthoCameraController.GetCamera());

	GE::Renderer2D::FillQuadColor({ 0.0f, 0.0f, -0.1f }, { 1.0f, 1.0f }, rotation, m_ShaderColor);
	GE::Renderer2D::FillQuadTexture({ 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f }, 0.0f, m_SpriteSheet);

	GE::Renderer2D::End();

	GE::Renderer2D::Start(m_OrthoCameraController.GetCamera());

	GE::Renderer2D::FillQuadSubTexture({ 5.0f, 0.0f, 0.1f }, { 1.0f, 1.0f }, 0.0f, m_Sprite);

	GE::Renderer2D::End();

}

void Sandbox2D::OnEvent(GE::Event& e)
{
	m_OrthoCameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
	// Note: Switch this to true to enable dockspace
	static bool dockingEnabled = true;
	if (dockingEnabled)
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

				if (ImGui::MenuItem("Exit")) GE::Application::GetApplication().Close();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");

	ImGui::Begin("Settings");
	
	GE::Renderer2D::Statistics stats = GE::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats: ");
	ImGui::Text("Draw Calls - %d", stats.DrawCalls);
	ImGui::Text("Quad Count - %d", stats.QuadCount);
	ImGui::Text("Vertices - %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices - %d", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Shader Color", glm::value_ptr(m_ShaderColor));
	
	ImGui::End();
	uint32_t textureID = m_SpriteSheet->GetID();
	ImGui::Image((void*)textureID, ImVec2{ 256.0f, 256.0f });
	ImGui::End();

	ImGui::End();
	}
	else
	{
		ImGui::Begin("Settings");

		GE::Renderer2D::Statistics stats = GE::Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats: ");
		ImGui::Text("Draw Calls - %d", stats.DrawCalls);
		ImGui::Text("Quad Count - %d", stats.QuadCount);
		ImGui::Text("Vertices - %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices - %d", stats.GetTotalIndexCount());

		ImGui::ColorEdit4("Shader Color", glm::value_ptr(m_ShaderColor));

		uint32_t textureID = m_SpriteSheet->GetID();
		ImGui::Image((void*)textureID, ImVec2{ 256.0f, 256.0f });
		ImGui::End();
	}
}

void Sandbox2D::OnWindowResize(GE::WindowResizeEvent& e)
{
	GE::RenderCommand::SetViewport(m_OrthoCameraController.GetCamera().GetPosition().x, m_OrthoCameraController.GetCamera().GetPosition().y, e.GetWidth(), e.GetHeight());

}