#include "GE/GEpch.h"

#include "VertexArray.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "Platform/OpenGL/VertexArray/OpenGLVertexArray.h"

namespace GE
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::InstanceAPI())
		{
		case Renderer::API::None:
			GE_CORE_ASSERT(false, "No Renderer API given!");
			break;
		case Renderer::API::OpenGL:
			return CreateRef<OpenGLVertexArray>();
			break;
		}

		GE_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

}