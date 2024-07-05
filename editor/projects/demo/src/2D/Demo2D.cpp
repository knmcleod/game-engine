#include "Demo2D.h"

Demo2D::Demo2D(const std::string& name) : Layer(name)
{
}

void Demo2D::OnAttach()
{
	GE::RenderCommand::Init();
}

void Demo2D::OnDetach()
{
	GE::RenderCommand::ShutDown();
}

void Demo2D::OnUpdate(GE::Timestep timestep)
{
	

}

void Demo2D::OnEvent(GE::Event& e)
{

}

void Demo2D::OnImGuiRender()
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

void Demo2D::OnWindowResize(GE::WindowResizeEvent& e)
{

}