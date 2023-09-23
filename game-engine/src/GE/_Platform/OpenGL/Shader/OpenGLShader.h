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

		//
		static GLenum ShaderDataTypeFromString(const std::string& type)
		{
			if (type == "vertex")
			{
				return GL_VERTEX_SHADER;
			}
			else if (type == "fragment" || type == "pixel")
			{
				return GL_FRAGMENT_SHADER;
			}
			GE_CORE_ASSERT(false, "Unknown Shader type!");
			return 0;
		}

		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		OpenGLShader(const std::string& path);
		virtual ~OpenGLShader();

		virtual const std::string& GetName() const override { return m_Name;  }

		//	Binds program using OpenGL
		virtual void Bind() const override;

		//	Unbinds program using OpenGL
		virtual void Unbind() const override;
			
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat(const std::string& name, const glm::vec1& value) override;
		virtual void SetInt(const std::string& name, const int value) override;
		
	private:
		uint32_t m_RendererID = 0;

		std::string m_Name;

		std::string ReadFile(const std::string& path);

		std::unordered_map<GLenum, std::string> Preprocess(const std::string& src);

		// Compiles shader using OpenGL
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSrc);
	
		// Uploads uniform int to renderer using OpenGL
		void UploadUniformInt(const std::string& name,
			int value);

		// Uploads uniform float to renderer using OpenGL
		void UploadUniformFloat(const std::string& name,
			const glm::vec1& vector);

		// Uploads uniform 2d float to renderer using OpenGL
		void UploadUniformFloat2(const std::string& name,
			const glm::vec2& vector);

		// Uploads uniform 3d float to renderer using OpenGL
		void UploadUniformFloat3(const std::string& name,
			const glm::vec3& vector);

		// Uploads uniform 4d float to renderer using OpenGL
		void UploadUniformFloat4(const std::string& name,
			const glm::vec4& vector);

		// Uploads uniform 4x4 matrix to renderer using OpenGL
		void UploadUniformMat4(const std::string& name,
			const glm::mat4& matrix);
	};
}