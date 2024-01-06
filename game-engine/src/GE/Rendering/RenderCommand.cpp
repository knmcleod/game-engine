#include "GE/GEpch.h"

#include "RenderCommand.h"

#include "Platform/OpenGL/RenderAPI/OpenGLRenderAPI.h"

namespace GE
{
	RenderAPI* RenderCommand::s_RenderAPI = new OpenGLRenderAPI();
}