#include "GEpch.h"

#include "Shader.h"
#include "GE/_Platform/OpenGL/Shader/OpenGLShader.h"

#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	Shader::~Shader()
	{
	}

	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return new OpenGLShader(vertexSrc, fragmentSrc);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}