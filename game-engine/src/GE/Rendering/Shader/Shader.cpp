#include "GE/GEpch.h"

#include "Shader.h"
#include "GE/_Platform/OpenGL/Shader/OpenGLShader.h"

#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	Shader::~Shader()
	{
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return (Ref<Shader>) new OpenGLShader(name, vertexSrc, fragmentSrc);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return (Ref<Shader>) new OpenGLShader(path);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader, const std::string& name)
	{
		if (name == "")
		{
			auto& name = shader->GetName();
			GE_CORE_ASSERT(!Exists(name), "Shader already exists!");
			m_Shaders[name] = shader;
		}
		else
		{
			GE_CORE_ASSERT(!Exists(name), "Shader already exists!");
			m_Shaders[name] = shader;
		}
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& path, const std::string& name)
	{
		auto shader = Shader::Create(path);
		Add(shader, name);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		GE_CORE_ASSERT(Exists(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

}