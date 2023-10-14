#include <GE/GE.h>
#include "GE/Core/EntryPoint.h"

#include "2D/Sandbox2D.h"

class Sandbox : public GE::Application
{
public:
	Sandbox()
	{
		PushLayer(new Sandbox2D());
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
