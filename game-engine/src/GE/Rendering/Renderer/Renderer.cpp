#include "GE/GEpch.h"

#include "GE/Rendering/Renderer/Renderer.h"
#include "2D/Renderer2D.h"

namespace GE
{

	void Renderer::Init()
	{
		Renderer2D::Init();
	}

	void Renderer::ShutDown()
	{
		Renderer2D::ShutDown();
	}

}
