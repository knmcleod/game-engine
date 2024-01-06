#include "Sandbox2D.h"

Sandbox2D::Sandbox2D(const std::string& name) : Layer(name)
{
}

void Sandbox2D::OnAttach()
{
	GE::RenderCommand::Init();
}

void Sandbox2D::OnDetach()
{
	GE::RenderCommand::ShutDown();
}

void Sandbox2D::OnUpdate(GE::Timestep timestep)
{
	

}

void Sandbox2D::OnEvent(GE::Event& e)
{

}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");

	GE::Renderer2D::Statistics stats = GE::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats: ");
	ImGui::Text("Draw Calls - %d", stats.DrawCalls);
	ImGui::Text("Quad Count - %d", stats.SpawnCount);
	ImGui::Text("Vertices - %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices - %d", stats.GetTotalIndexCount());

	ImGui::End();
}

void Sandbox2D::OnWindowResize(GE::WindowResizeEvent& e)
{

}