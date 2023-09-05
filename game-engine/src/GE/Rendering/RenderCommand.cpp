#include "GE/GEpch.h"

#include "GE/Rendering/RenderCommand.h"
#include "GE/_Platform/OpenGL/RenderAPI/OpenGLRenderAPI.h"

namespace GE
{
	RenderAPI* RenderCommand::s_RenderAPI = new OpenGLRenderAPI();
}