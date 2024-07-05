#pragma once

#include <GE/GE.h>

#include <glm/gtc/type_ptr.hpp>

class Demo2D : public GE::Layer
{
public:
	Demo2D(const std::string& name = "SandboxLayer2D");
	virtual ~Demo2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(GE::Timestep timestep) override;
	virtual void OnEvent(GE::Event& e) override;
	virtual void OnImGuiRender() override;
	virtual void OnWindowResize(GE::WindowResizeEvent& e) override;

};
