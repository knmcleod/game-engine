#pragma once

#include "GE/Rendering/Shader/Shader.h"

namespace GE
{
	struct BufferElements
	{
		std::string Name = "BufferElement";
		Shader::ShaderDataType Type;
		uint32_t Offset = 0;
		uint32_t Size = 0;
		bool Normalized = false;

		BufferElements(Shader::ShaderDataType type, const std::string& name) : Name(name), Type(type), Size(Shader::ShaderDataTypeSize(type)), Offset(0), Normalized(false)
		{
			
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case Shader::ShaderDataType::Float:
				return 1;
			case Shader::ShaderDataType::Float2:
				return 2;
			case Shader::ShaderDataType::Float3:
				return 3;
			case Shader::ShaderDataType::Float4:
				return 4;
			case Shader::ShaderDataType::Mat3:
				return 3 * 3;
			case Shader::ShaderDataType::Mat4:
				return 4 * 4;
			case Shader::ShaderDataType::Int:
				return 1;
			case Shader::ShaderDataType::Int2:
				return 2;
			case Shader::ShaderDataType::Int3:
				return 3;
			case Shader::ShaderDataType::Int4:
				return 4;
			case Shader::ShaderDataType::Bool:
					return 1;
			}
			GE_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout()
		{

		}

		BufferLayout(const std::initializer_list<BufferElements>& elements) : m_Elements(elements)
		{ 
			CalculateOffsetAndStride();
		}

		inline const std::vector<BufferElements> GetElements() { return m_Elements; }
		inline const uint32_t GetStride() { return m_Stride; }

		std::vector<BufferElements>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElements>::iterator end() { return m_Elements.end(); }
	private:
		std::vector<BufferElements> m_Elements;
		uint32_t m_Stride = 0;

		// Calculates offset and stride based on elements size
		void CalculateOffsetAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		// Binds buffer
		virtual void Bind() const = 0;

		// Unbinds buffer
		virtual void Unbind() const = 0;

		virtual inline BufferLayout& GetLayout() = 0;
		virtual inline void SetLayout(const BufferLayout& layout) = 0;

		static Ref<VertexBuffer> Create(uint32_t size, float* vertices);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		// Binds buffer
		virtual void Bind() const = 0;

		// Unbinds buffer
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(uint32_t size, uint32_t* indices);
	};

}