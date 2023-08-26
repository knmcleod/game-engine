#include "GEpch.h"

#include "Shader.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace GE
{
	Shader::~Shader()
	{
	}

	void Shader::Bind() const
	{
	}

	void Shader::Unbind() const
	{
	}

	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case Renderer::RendererAPI::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case Renderer::RendererAPI::OpenGL:
			return new OpenGLShader(vertexSrc, fragmentSrc);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}