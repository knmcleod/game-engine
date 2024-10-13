#include "GE/GEpch.h"

#include "Shader.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "Platform/OpenGL/Shader/OpenGLShader.h"


namespace GE
{
	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::Get()->GetAPI())
		{
		case Renderer::API::None:
			GE_CORE_ASSERT(false, "No Renderer API given!");
			return nullptr;
			break;
		case Renderer::API::OpenGL:
			return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Renderer::Get()->GetAPI())
		{
		case Renderer::API::None:
			GE_CORE_ASSERT(false, "No Renderer API given!");
			return nullptr;
			break;
		case Renderer::API::OpenGL:
			return CreateRef<OpenGLShader>(path);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

#pragma region ShaderLibrary

	void ShaderLibrary::Add(Ref<Shader> shader, const std::string& name)
	{
		if (name == std::string())
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

#pragma endregion
}