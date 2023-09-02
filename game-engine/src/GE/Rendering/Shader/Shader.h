#pragma once
#include <string>
#include <glm/glm.hpp>

namespace GE
{
	class Shader
	{
	public:
		~Shader();

		enum class ShaderDataType
		{
			None,
			Float, Float2, Float3, Float4,
			Mat3, Mat4,
			Int, Int2, Int3, Int4,
			Bool
		};

		static uint32_t ShaderDataTypeSize(ShaderDataType type)
		{
			switch (type)
			{
			case ShaderDataType::Float:
				return 4;
			case ShaderDataType::Float2:
				return 4 * 2;
			case ShaderDataType::Float3:
				return 4 * 3;
			case ShaderDataType::Float4:
				return 4 * 4;
			case ShaderDataType::Mat3:
				return 4 * 3 * 3;
			case ShaderDataType::Mat4:
				return 4 * 4 * 4;
			case ShaderDataType::Int:
				return 4;
			case ShaderDataType::Int2:
				return 4 * 2;
			case ShaderDataType::Int3:
				return 4 * 3;
			case ShaderDataType::Int4:
				return 4 * 4;
			case ShaderDataType::Bool:
				return 1;
			}

			GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}

		static Shader* Create(const std::string& vertexSrc,
			const std::string& fragmentSrc);
	
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void UploadUniformInt(const std::string& name, int value) = 0;

		virtual void UploadUniformFloat(const std::string& name,
			const glm::vec1& vector) = 0;

		virtual void UploadUniformFloat2(const std::string& name,
			const glm::vec2& vector) = 0;

		virtual void UploadUniformFloat3(const std::string& name,
			const glm::vec3& vector) = 0;

		virtual void UploadUniformFloat4(const std::string& name,
			const glm::vec4& vector) = 0;

		virtual void UploadUniformMat4(const std::string& name,
			const glm::mat4& matrix) = 0;

	};
}