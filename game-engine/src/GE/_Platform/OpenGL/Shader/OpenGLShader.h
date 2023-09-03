#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include "GE/Rendering/Shader/Shader.h"

namespace GE
{
	class OpenGLShader : public Shader
	{
	public:
		// Converts Shader data type to OpenGL type
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
		virtual ~OpenGLShader();

		//	Binds program using OpenGL
		virtual void Bind() const override;

		//	Unbinds program using OpenGL
		virtual void Unbind() const override;

		// Uploads uniform int to renderer using OpenGL
		virtual void UploadUniformInt(const std::string& name,
			int value) override;

		// Uploads uniform float to renderer using OpenGL
		virtual void UploadUniformFloat(const std::string& name,
			const glm::vec1& vector) override;

		// Uploads uniform 2d float to renderer using OpenGL
		virtual void UploadUniformFloat2(const std::string& name,
			const glm::vec2& vector) override;

		// Uploads uniform 3d float to renderer using OpenGL
		virtual void UploadUniformFloat3(const std::string& name,
			const glm::vec3& vector) override;

		// Uploads uniform 4d float to renderer using OpenGL
		virtual void UploadUniformFloat4(const std::string& name,
			const glm::vec4& vector) override;

		// Uploads uniform 4x4 matrix to renderer using OpenGL
		virtual void UploadUniformMat4(const std::string& name,
			const glm::mat4& matrix) override;

	private:
		uint32_t m_RendererID = 0;
	};
}