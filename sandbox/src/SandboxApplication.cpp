#include <GE.h>

class Sandbox : public GE::Application
{
public:
	Sandbox()
	{

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
