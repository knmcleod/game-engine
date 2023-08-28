#pragma once

#include <glad/glad.h>

#include "GE/Rendering/Shader/Shader.h"

namespace GE
{
	class OpenGLShader : public Shader
	{
	public:
		static GLenum ShaderDataTypeToOpenGLBaseType(Shader::ShaderDataType type)
		{
			switch (type)
			{
			case Shader::ShaderDataType::Float:
				return GL_FLOAT;
				break;
			case Shader::ShaderDataType::Float2:
				return GL_FLOAT;
				break;
			case Shader::ShaderDataType::Float3:
				return GL_FLOAT;
				break;
			case Shader::ShaderDataType::Float4:
				return GL_FLOAT;
				break;
			case Shader::ShaderDataType::Mat3:
				return GL_FLOAT;
				break;
			case Shader::ShaderDataType::Mat4:
				return GL_FLOAT;
				break;
			case Shader::ShaderDataType::Int:
				return GL_INT;
				break;
			case Shader::ShaderDataType::Int2:
				return GL_INT;
				break;
			case Shader::ShaderDataType::Int3:
				return GL_INT;
				break;
			case Shader::ShaderDataType::Int4:
				return GL_INT;
				break;
			case Shader::ShaderDataType::Bool:
				return GL_BOOL;
				break;
			}
			GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}

		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;
	private:
		uint32_t m_RendererID = 0;
	};
}