#pragma once

#include "GE/Rendering/Shader/Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

namespace GE
{
	class OpenGLShader : public Shader
	{
	public:
		// Converts Shader data type to OpenGL type
		static GLenum ShaderDataTypeToOpenGLBaseType(Shader::DataType type)
		{
			switch (type)
			{
			case Shader::DataType::Float:
				return GL_FLOAT;
				break;
			case Shader::DataType::Float2:
				return GL_FLOAT;
				break;
			case Shader::DataType::Float3:
				return GL_FLOAT;
				break;
			case Shader::DataType::Float4:
				return GL_FLOAT;
				break;
			case Shader::DataType::Mat3:
				return GL_FLOAT;
				break;
			case Shader::DataType::Mat4:
				return GL_FLOAT;
				break;
			case Shader::DataType::Int:
				return GL_INT;
				break;
			case Shader::DataType::Int2:
				return GL_INT;
				break;
			case Shader::DataType::Int3:
				return GL_INT;
				break;
			case Shader::DataType::Int4:
				return GL_INT;
				break;
			case Shader::DataType::Bool:
				return GL_BOOL;
				break;
			}
			GE_CORE_ASSERT(false, "Unknown DataType!");
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
		~OpenGLShader() override;

		const std::string& GetName() const override { return m_Name;  }
		
		// Bind and Unbind handled
		void SetMat4(const std::string& name, const glm::mat4& value) override;
		// Bind and Unbind handled
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		// Bind and Unbind handled
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		// Bind and Unbind handled
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		// Bind and Unbind handled
		void SetFloat(const std::string& name, const float& value) override;
		// Bind and Unbind handled
		void SetInt(const std::string& name, const int value) override;
		// Bind and Unbind handled
		void SetIntArray(const std::string& name, const int* values, uint32_t count) override;

		void Bind() const override;
		void Unbind() const override;
		
	private:
		std::string ReadFile(const std::string& path);

		std::unordered_map<GLenum, std::string> Preprocess(const std::string& src);

		// Compiles shader using OpenGL
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSrc);
		
		// Uploads uniform int array to renderer using OpenGL
		// Bind and Unbind handled
		void UploadUniformIntArray(const std::string& name,
			const int* values, uint32_t count);

		// Uploads uniform int to renderer using OpenGL
		// Bind and Unbind handled
		void UploadUniformInt(const std::string& name,
			int value);

		// Uploads uniform float to renderer using OpenGL
		// Bind and Unbind handled
		void UploadUniformFloat(const std::string& name,
			const float& vector);

		// Uploads uniform 2d float to renderer using OpenGL
		// Bind and Unbind handled
		void UploadUniformFloat2(const std::string& name,
			const glm::vec2& vector);

		// Uploads uniform 3d float to renderer using OpenGL
		// Bind and Unbind handled
		void UploadUniformFloat3(const std::string& name,
			const glm::vec3& vector);

		// Uploads uniform 4d float to renderer using OpenGL
		// Bind and Unbind handled
		void UploadUniformFloat4(const std::string& name,
			const glm::vec4& vector);

		// Uploads uniform 4x4 matrix to renderer using OpenGL
		// Bind and Unbind handled
		void UploadUniformMat4(const std::string& name,
			const glm::mat4& matrix);
	private:
		uint32_t m_RendererID = 0;

		std::string m_Name;
	};
}