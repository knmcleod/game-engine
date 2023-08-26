#include <GE.h>

class ExampleLayer : public GE::Layer
{
public:
	ExampleLayer() : GE::Layer("Example")
	{
		
	}

	void OnUpdate() override
	{
	
	}

	void OnEvent(GE::Event& e) override
	{
		
	}
};

class Sandbox : public GE::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
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
