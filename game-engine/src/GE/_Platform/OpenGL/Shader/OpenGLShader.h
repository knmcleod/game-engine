#pragma once
#include "GE/Rendering/Shader/Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace GE
{
	class OpenGLShader : public Shader
	{
	public:
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
		virtual void SetFloat(const std::string& name, const float& value) override;
		virtual void SetInt(const std::string& name, const int value) override;
		virtual void SetIntArray(const std::string& name, const int* values, uint32_t count) override;
	
	private:

		std::string ReadFile(const std::string& path);
		std::unordered_map<GLenum, std::string> Preprocess(const std::string& src);

		// Compiles shader using OpenGL
		//void Compile(const std::unordered_map<GLenum, std::string>& shaderSrc);

		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		// Prints information about OpenGL Shader
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);

		// Uploads uniform int array to renderer using OpenGL
		void UploadUniformIntArray(const std::string& name, const int* values, uint32_t count);
		// Uploads uniform int to renderer using OpenGL
		void UploadUniformInt(const std::string& name, int value);
		// Uploads uniform float to renderer using OpenGL
		void UploadUniformFloat(const std::string& name, const float& vector);
		// Uploads uniform 2d float to renderer using OpenGL
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vector);
		// Uploads uniform 3d float to renderer using OpenGL
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vector);
		// Uploads uniform 4d float to renderer using OpenGL
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vector);
		// Uploads uniform 4x4 matrix to renderer using OpenGL
		void UploadUniformMat4(const std::string& name,const glm::mat4& matrix);

	private:
		uint32_t m_RendererID = 0;

		std::string m_Name;
		std::string m_FilePath;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;

	};
}