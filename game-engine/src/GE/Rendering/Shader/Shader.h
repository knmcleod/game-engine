#pragma once
#include <string>
#include <glm/glm.hpp>

#include "GE/Core/Core.h"

namespace GE
{
	class Shader
	{
	public:
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

		virtual ~Shader();

		virtual const std::string& GetName() const = 0;

		//	Returns created Shader given vertex and fragment sources
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc,
			const std::string& fragmentSrc);
		
		//	Returns created Shader given path
		static Ref<Shader> Create(const std::string& path);

		// Binds program
		virtual void Bind() const = 0;
		// Uninds program
		virtual void Unbind() const = 0;

		// Uploads uniform int to renderer
		virtual void UploadUniformInt(const std::string& name,
			int value) = 0;

		// Uploads uniform float to renderer
		virtual void UploadUniformFloat(const std::string& name,
			const glm::vec1& vector) = 0;

		// Uploads uniform 2d float to renderer
		virtual void UploadUniformFloat2(const std::string& name,
			const glm::vec2& vector) = 0;

		// Uploads uniform 3d float to renderer
		virtual void UploadUniformFloat3(const std::string& name,
			const glm::vec3& vector) = 0;

		// Uploads uniform 4d float to renderer
		virtual void UploadUniformFloat4(const std::string& name,
			const glm::vec4& vector) = 0;

		// Uploads uniform 4x4 matrix to renderer
		virtual void UploadUniformMat4(const std::string& name,
			const glm::mat4& matrix) = 0;

	};

	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader, const std::string& name = "");
		Ref<Shader> Load(const std::string& path, const std::string& name = "");

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}