#include "GE/GEpch.h"

#include "OpenGLShader.h"

namespace GE
{
	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		std::unordered_map<GLenum, std::string> source;
		source[GL_VERTEX_SHADER] = vertexSrc;
		source[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(source);
	}

	OpenGLShader::OpenGLShader(const std::string& path)
	{
		Compile(Preprocess(ReadFile(path)));

		// Extracts name from path
		auto lastSlash = path.find_last_of("/\\");
		lastSlash = (lastSlash == std::string::npos ? 0 : lastSlash + 1);

		auto lastDot = path.rfind('.');
		auto count = (lastDot == std::string::npos ? path.size() - lastSlash : lastDot - lastSlash);
		m_Name = path.substr(lastSlash, count);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat(const std::string& name, const float& value)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetInt(const std::string& name, const int value)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, const int* values, uint32_t count)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::Bind() const
	{
		GE_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		GE_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	std::string OpenGLShader::ReadFile(const std::string& path)
	{
		GE_PROFILE_FUNCTION();

		std::string result = std::string();
		std::ifstream in(path, std::ios::in | std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg()); // resizes result to end of file
			in.seekg(0, std::ios::beg);

			in.read(&result[0], result.size());

			in.close();
		}
		else
		{
			GE_CORE_ERROR("Could not open file '{0}'", path);

		}
		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::Preprocess(const std::string& shaderSrc)
	{
		GE_PROFILE_FUNCTION();
		std::unordered_map<GLenum, std::string> processedSrc;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t position = shaderSrc.find(typeToken, 0);

		while (position != std::string::npos)
		{
			size_t eol = shaderSrc.find_first_of("\r\n", position);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t begin = position + typeTokenLength + 1;
			std::string type = shaderSrc.substr(begin, eol - begin);
			GE_CORE_ASSERT(OpenGLShader::ShaderDataTypeFromString(type), "Invalid shader type specified!");

			size_t nextLinePos = shaderSrc.find_first_not_of("\r\n", eol);
			position = shaderSrc.find(typeToken, nextLinePos);
			processedSrc[ShaderDataTypeFromString(type)] = shaderSrc.substr(nextLinePos,
				position - (nextLinePos == std::string::npos ? shaderSrc.size() - 1 : nextLinePos));
		}

		return processedSrc;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& processedSrc)
	{
		GE_PROFILE_FUNCTION();

		// Get a program object.
		GLuint program = glCreateProgram();

		GE_CORE_ASSERT(processedSrc.size() <= 2, "Only TWO or less Shaders are supported right now.");

		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;

		for (auto& [type, src] : processedSrc)
		{
			uint32_t shader = glCreateShader(type);

			// Send the vertex shader source code to GL
			// Note that std::string's .c_str is NULL character terminated.
			const char* shaderSource = src.c_str();
			glShaderSource(shader, 1, &shaderSource, 0);

			// Compile the vertex shader
			glCompileShader(shader);

			int isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				int maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<char> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// We don't need the shader anymore.
				glDeleteShader(shader);

				GE_CORE_ERROR("{0}", infoLog.data());
				GE_CORE_ASSERT(false, "OpenGLShader Compilation Failure!");
				break;
			}

			// Attach our shaders to our program
			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		// Shaders are successfully compiled.
		// Now time to link them together into a program.

		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		int isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			int maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<char> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);

			for (auto shader : glShaderIDs)
			{
				// Don't leak shaders either.
				glDeleteShader(shader);
			}

			GE_CORE_ERROR("	{0}	", infoLog.data());
			GE_CORE_ASSERT(false, "OpenGLShader Link Failure!");
			return;
		}

		// Always detach shaders after a successful link.
		for (auto shader : glShaderIDs)
		{
			glDetachShader(program, shader);
		}

		m_RendererID = program;
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, const int* values, uint32_t count)
	{
		this->Bind();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
		this->Unbind();
	}
	
	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		this->Bind();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
		this->Unbind();
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const float& value)
	{
		this->Bind();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
		this->Unbind();
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vector)
	{
		this->Bind();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, vector.x, vector.y);
		this->Unbind();
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vector)
	{
		this->Bind();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, vector.x, vector.y, vector.z);
		this->Unbind();
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vector)
	{
		this->Bind();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
		this->Unbind();
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		this->Bind();
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		this->Unbind();
	}

}