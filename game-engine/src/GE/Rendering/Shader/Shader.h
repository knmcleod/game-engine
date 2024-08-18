#pragma once
#include <string>
#include <glm/glm.hpp>

namespace GE
{
	class Shader
	{
	public:
		// TODO: Math wrapper
		enum class DataType
		{
			None,
			Float, Float2, Float3, Float4,
			Mat3, Mat4,
			Int, Int2, Int3, Int4,
			Bool
		};

		static uint32_t GetDataTypeSize(DataType type)
		{
			switch (type)
			{
			case DataType::Float:
				return 4;
			case DataType::Float2:
				return 4 * 2;
			case DataType::Float3:
				return 4 * 3;
			case DataType::Float4:
				return 4 * 4;
			case DataType::Mat3:
				return 4 * 3 * 3;
			case DataType::Mat4:
				return 4 * 4 * 4;
			case DataType::Int:
				return 4;
			case DataType::Int2:
				return 4 * 2;
			case DataType::Int3:
				return 4 * 3;
			case DataType::Int4:
				return 4 * 4;
			case DataType::Bool:
				return 1;
			}

			GE_CORE_ASSERT(false, "Unknown DataType!");
			return 0;
		}

		static Ref<Shader> Create(const std::string& name, 
			const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& path);

		virtual ~Shader() = default;

		virtual const std::string& GetName() const = 0;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat(const std::string& name, const float& value) = 0;
		virtual void SetInt(const std::string& name, const int value) = 0;
		virtual void SetIntArray(const std::string& name, const int* values, uint32_t count) = 0;

		// Binds program
		virtual void Bind() const = 0;
		// Uninds program
		virtual void Unbind() const = 0;

	};

	// TODO: Remove, Move Shader to Assets
	class ShaderLibrary
	{
	public:
		void Add(Ref<Shader> shader, const std::string& name = "New Shader");
		Ref<Shader> Load(const std::string& path, const std::string& name = "New Shader");

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}