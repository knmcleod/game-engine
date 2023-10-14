#pragma once

#include "GE/GE.h"

#include <glm/gtc/type_ptr.hpp>

class Sandbox2D : public GE::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(GE::Timestep timestep) override;
	virtual void OnEvent(GE::Event& e) override;
	virtual void OnImGuiRender() override;

private:
	GE::OrthographicCameraController m_OrthoCameraController;

	glm::vec4 m_ShaderColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
