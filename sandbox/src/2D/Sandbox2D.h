#pragma once

#include <GE/GE.h>

#include <glm/gtc/type_ptr.hpp>

class Sandbox2D : public GE::Layer
{
public:
	Sandbox2D(const std::string& name = "SandboxLayer2D");
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(GE::Timestep timestep) override;
	virtual void OnEvent(GE::Event& e) override;
	virtual void OnImGuiRender() override;
	virtual void OnWindowResize(GE::WindowResizeEvent& e) override;

private:
	GE::OrthographicCameraController m_OrthoCameraController;

	// Temp
	glm::vec4 m_ShaderColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	GE::Ref<GE::Texture2D> m_Texture;
	GE::Ref<GE::Texture2D> m_SpriteSheet;
	GE::Ref<GE::SubTexture2D> m_Sprite;
};
