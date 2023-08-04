#include <GE.h>

class ExampleLayer : public GE::Layer
{
public:
	ExampleLayer() : Layer("Example")
	{

	}

	void OnUpdate() override
	{
		//GE_INFO("ExampleLayer::Update");

		if (GE::Input::IsKeyPressed(GE_KEY_TAB))
			GE_TRACE("Tab key is pressed!");
	}

	void OnEvent(GE::Event& e) override
	{
		GE_TRACE("{0}", e);
	}
};

class Sandbox : public GE::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new GE::ImGuiLayer());
	}

	~Sandbox()
	{

	}

private:

};

GE::Application* GE::CreateApplication()
{
	return new Sandbox();
}
