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

	void OpenGLShader::SetFloat(const std::string& name, const glm::vec1& value)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetInt(const std::string& name, const int value)
	{
		GE_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	std::string OpenGLShader::ReadFile(const std::string& path)
	{
		GE_PROFILE_FUNCTION();

		std::string result;
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

	std::unordered_map<GLenum, std::string> OpenGLShader::Preprocess(const std::string& src)
	{
		GE_PROFILE_FUNCTION();
		std::unordered_map<GLenum, std::string> shaderSrc;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t position = src.find(typeToken, 0);

		while (position != std::string::npos)
		{
			size_t eol = src.find_first_of("\r\n", position);
			GE_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t begin = position + typeTokenLength + 1;
			std::string type = src.substr(begin, eol - begin);
			GE_CORE_ASSERT(OpenGLShader::ShaderDataTypeFromString(type), "Invalid shader type specified!");

			size_t nextLinePos = src.find_first_not_of("\r\n", eol);
			position = src.find(typeToken, nextLinePos);
			shaderSrc[ShaderDataTypeFromString(type)] = src.substr(nextLinePos,
				position - (nextLinePos == std::string::npos ? src.size() - 1 : nextLinePos));
		}

		return shaderSrc;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSrc)
	{
		GE_PROFILE_FUNCTION();

		// Get a program object.
		GLuint program = glCreateProgram();

		GE_CORE_ASSERT(shaderSrc.size() <= 2, "Only TWO Shaders are supported right now.");

		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;

		for (auto& kv : shaderSrc)
		{
			GLenum shaderType = kv.first;
			const std::string source = kv.second;

			uint32_t shader = glCreateShader(shaderType);

			// Send the vertex shader source code to GL
			// Note that std::string's .c_str is NULL character terminated.
			const char* shaderSource = source.c_str();
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
	
	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const glm::vec1& vector)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, vector.x);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vector)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, vector.x, vector.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vector)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, vector.x, vector.y, vector.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vector)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}