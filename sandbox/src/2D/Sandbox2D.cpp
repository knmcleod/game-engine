#include "Sandbox2D.h"

Sandbox2D::Sandbox2D(const std::string& name) : Layer(name), m_OrthoCameraController(GE::Application::GetApplication().GetWindow().GetWidth(), GE::Application::GetApplication().GetWindow().GetHeight())
{
}

void Sandbox2D::OnAttach()
{
	GE::RenderCommand::SetClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
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
	GE::Renderer2D::FillQuadTexture({ 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f }, 0.0f, GE::Texture2D::Create("assets/textures/image.jpg"));

	GE::Renderer2D::End();

}

void Sandbox2D::OnEvent(GE::Event& e)
{
	m_OrthoCameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	
	GE::Renderer2D::Statistics stats = GE::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats: ");
	ImGui::Text("Draw Calls - %d", stats.DrawCalls);
	ImGui::Text("Quad Count - %d", stats.QuadCount);
	ImGui::Text("Vertices - %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices - %d", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Shader Color", glm::value_ptr(m_ShaderColor));
	
	ImGui::End();
}

void Sandbox2D::OnWindowResize(GE::WindowResizeEvent& e)
{
	GE::RenderCommand::SetViewport(m_OrthoCameraController.GetCamera().GetPosition().x, m_OrthoCameraController.GetCamera().GetPosition().y, e.GetWidth(), e.GetHeight());

}