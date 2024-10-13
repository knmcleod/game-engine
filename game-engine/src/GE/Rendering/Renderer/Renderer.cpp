#include "GE/GEpch.h"

#include "Renderer.h"
#include "Platform/OpenGL/Renderer/OpenGLRenderer.h"

namespace GE
{
	Ref<Renderer> Renderer::s_Instance = nullptr;

	void Renderer::Create(Renderer::API api)
	{
		switch (api)
		{
		case API::None:
			GE_CORE_ASSERT(false, "Cannot create Renderer. API is None.");
			break;
		case API::OpenGL:
			s_Instance = CreateRef<OpenGLRenderer>();
			s_Instance->CreateData();
			break;
		default:
			GE_CORE_ERROR("Cannot create Renderer. API is Unknown.");
			break;
		}
	}
}