#pragma once

#include "GE/Rendering/Shader/Shader.h"

namespace GE
{
	class VertexBuffer
	{
		friend class VertexArray;
	public:
		struct Elements
		{
			std::string Name = "BufferElement";
			Shader::DataType Type;
			uint32_t Count = 0;
			uint32_t Offset = 0;
			uint32_t Size = 0;
			bool Normalized = false;

			Elements(Shader::DataType type, const std::string& name) 
				: Name(name), Type(type), Size(Shader::GetDataTypeSize(type)), Offset(0), Normalized(false)
			{

			}

			uint32_t GetComponentCount() const
			{
				switch (Type)
				{
				case Shader::DataType::Float:
					return 1;
				case Shader::DataType::Float2:
					return 2;
				case Shader::DataType::Float3:
					return 3;
				case Shader::DataType::Float4:
					return 4;
				case Shader::DataType::Mat3:
					return 3 * 3;
				case Shader::DataType::Mat4:
					return 4 * 4;
				case Shader::DataType::Int:
					return 1;
				case Shader::DataType::Int2:
					return 2;
				case Shader::DataType::Int3:
					return 3;
				case Shader::DataType::Int4:
					return 4;
				case Shader::DataType::Bool:
					return 1;
				}
				GE_CORE_ASSERT(false, "Unknown DataType!");
				return Count;
			}
		};

		struct Layout
		{
			friend class VertexBuffer;
		public:
			Layout() = default;

			Layout(const std::initializer_list<Elements>& elements) : m_Elements(elements)
			{
				CalculateOffsetAndStride();
			}

			inline const std::vector<Elements>& GetElements() { return m_Elements; }
			inline const uint32_t& GetStride() { return m_Stride; }

			std::vector<Elements>::iterator begin() { return m_Elements.begin(); }
			std::vector<Elements>::iterator end() { return m_Elements.end(); }
		private:
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
		private:
			std::vector<Elements> m_Elements;
			uint32_t m_Stride = 0;
		};

		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(uint32_t size, float* vertices);

		virtual ~VertexBuffer() = default;

		virtual inline Layout& GetLayout() = 0;
		virtual inline void SetLayout(const Layout& layout) = 0;
		
		virtual void SetData(const void* data, uint32_t size) = 0;
		
		// Binds buffer
		virtual void Bind() const = 0;
		// Unbinds buffer
		virtual void Unbind() const = 0;
	};

}