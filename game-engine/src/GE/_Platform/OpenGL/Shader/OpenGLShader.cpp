#include "GE/GEpch.h"

#include "OpenGLShader.h"
#include "GE/Core/Time/Time.h"

//#include <shaderc/shaderc.hpp>
//#include <spirv_cross/spirv_cross.hpp>
//#include <spirv_cross/spirv_glsl.hpp>

namespace GE
{
	namespace Utils
	{
		/*
		* Returns Shader cache for OpenGL
		*/
		static const char* GetOpenGLCacheDir()
		{
			return "assets/cache/shader/opengl";
		}

		static void CreateOpenGLCacheDir()
		{
			std::string cacheDir = GetOpenGLCacheDir();
			if (!std::filesystem::exists(cacheDir))
				std::filesystem::create_directories(cacheDir);
		}

		static const char* GLShaderStageCachedOpenGL(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER: return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER: return ".cached_opengl.frag";
			}
			GE_CORE_ASSERT(false, "Unknown GLShader Stage for OpenGL.");
			return "";
		}

		static const char* GLShaderStageCachedVulkan(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER: return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER: return ".cached_vulkan.frag";
			}
			GE_CORE_ASSERT(false, "Unknown GLShader Stage for Vulkan.");
			return "";
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			GE_CORE_ASSERT(false, "Unknown GL Shader Stage. Unable to convert to string.");
			return nullptr;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			GE_CORE_ASSERT(false, "Unknown GL Shader Stage. Unable to convert to ShaderC.");
			return (shaderc_shader_kind)0;
		}

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

	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		std::unordered_map<GLenum, std::string> source;
		source[GL_VERTEX_SHADER] = vertexSrc;
		source[GL_FRAGMENT_SHADER] = fragmentSrc;
		
		CompileOrGetVulkanBinaries(source);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
	}

	OpenGLShader::OpenGLShader(const std::string& path) : m_FilePath(path)
	{
		GE_PROFILE_FUNCTION();

		Utils::CreateOpenGLCacheDir();

		std::string source = ReadFile(path);
		auto shaderSources = Preprocess(source);

		{
			GE_PROFILE_SCOPE();

			Timer timer("Shader Creation");

			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();

			timer.Stop();
			GE_CORE_INFO("Shader creation took {0} ms", timer.GetMilliSeconds());
		}

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

#pragma region Set Functions

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

#pragma endregion

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
			GE_CORE_ASSERT(Utils::ShaderDataTypeFromString(type), "Invalid shader type specified!");

			size_t nextLinePos = src.find_first_not_of("\r\n", eol);
			position = src.find(typeToken, nextLinePos);
			shaderSrc[Utils::ShaderDataTypeFromString(type)] = src.substr(nextLinePos,
				position - (nextLinePos == std::string::npos ? src.size() - 1 : nextLinePos));
		}

		return shaderSrc;
	}

	/*void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSrc)
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
	}*/

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GE_PROFILE_FUNCTION();

		GLuint program = glCreateProgram();

		shaderc::Compiler shaderCCompiler;
		shaderc::CompileOptions shaderCOptions;

		shaderCOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		
		const bool optimize = true;
		if (optimize)
			shaderCOptions.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDir = Utils::GetOpenGLCacheDir();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
	
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDir / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkan(stage));

			std::ifstream input(cachedPath, std::ios::in | std::ios::binary);

			if (input.is_open())
			{
				input.seekg(0, std::ios::end);
				auto size = input.tellg();
				input.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				input.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = shaderCCompiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage),
																							m_FilePath.c_str(), shaderCOptions);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					GE_CORE_ERROR(module.GetErrorMessage());
					GE_CORE_ASSERT(false, "Compiling glsl to SPV failed.");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream output(cachedPath, std::ios::out | std::ios::binary);
				if (output.is_open())
				{
					auto& data = shaderData[stage];
					output.write((char*)data.data(), data.size() * sizeof(uint32_t));
					output.flush();
					output.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler shaderCCompiler;
		shaderc::CompileOptions shaderCOptions;
		shaderCOptions.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = false;
		if (optimize)
			shaderCOptions.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDir = Utils::GetOpenGLCacheDir();

		shaderData.clear();
		m_OpenGLSourceCode.clear();

		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDir / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGL(stage));

			std::ifstream input(cachedPath, std::ios::in | std::ios::binary);
			if (input.is_open())
			{
				input.seekg(0, std::ios::end);
				auto size = input.tellg();
				input.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				input.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile(); // Compiling Vulkan spirv into OpenGL glsl
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = shaderCCompiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					GE_CORE_ERROR(module.GetErrorMessage());
					GE_CORE_ASSERT(false, "Compiling glsl to SPV failed.");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream output(cachedPath, std::ios::out | std::ios::binary);
				if (output.is_open())
				{
					auto& data = shaderData[stage];
					output.write((char*)data.data(), data.size() * sizeof(uint32_t));
					output.flush();
					output.close();
				}
			}

		}
	}

	void OpenGLShader::CreateProgram()
	{
		GE_PROFILE_FUNCTION();

		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			GE_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		GE_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
		GE_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		GE_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		GE_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			GE_CORE_TRACE("  {0}", resource.name);
			GE_CORE_TRACE("    Size = {0}", bufferSize);
			GE_CORE_TRACE("    Binding = {0}", binding);
			GE_CORE_TRACE("    Members = {0}", memberCount);
		}
	}

#pragma region Upload Functions
	void OpenGLShader::UploadUniformIntArray(const std::string& name, const int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const float& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
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

#pragma endregion

}