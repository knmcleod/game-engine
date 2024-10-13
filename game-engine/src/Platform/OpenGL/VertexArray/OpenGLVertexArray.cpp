#include "GE/GEpch.h"

#include "OpenGLVertexArray.h"

#include "Platform/OpenGL/Shader/OpenGLShader.h"

namespace GE
{
	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		GE_PROFILE_FUNCTION();
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		GE_PROFILE_FUNCTION();
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(Ref<VertexBuffer> vertexBuffer)
	{
		GE_PROFILE_FUNCTION();
		auto& layout = vertexBuffer->GetLayout();
		GE_CORE_ASSERT(layout.GetElements().size(), "Vertex Buffer has no Layout.");

		this->Bind();
		vertexBuffer->Bind();

		uint32_t index = 0;
		
		for (auto& element : layout)
		{
			switch (element.Type)
			{
			case Math::Type::Float:
			case Math::Type::Float2:
			case Math::Type::Float3:
			case Math::Type::Float4:
			{
				glEnableVertexAttribArray(index);
				glVertexAttribPointer(index,
					element.GetComponentCount(),
					OpenGLShader::MathTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)(UINT_PTR)element.Offset);
				index++;
				break;
			}
			case Math::Type::Int:
			case Math::Type::Int2:
			case Math::Type::Int3:
			case Math::Type::Int4:
			case Math::Type::Bool:
			{
				glEnableVertexAttribArray(index);
				glVertexAttribIPointer(index,
					element.GetComponentCount(),
					OpenGLShader::MathTypeToOpenGLBaseType(element.Type),
					layout.GetStride(),
					(const void*)(UINT_PTR)element.Offset);
				index++;
				break;
			}
			case Math::Type::Mat3:
			case Math::Type::Mat4:
			{
				uint8_t count = element.GetComponentCount();
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index,
						count,
						OpenGLShader::MathTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(element.Offset + sizeof(float) * count * i));
					index++;
				}
				break;
			}
			default:
				GE_CORE_ASSERT(false, "Unknown Vertex Buffer Layout Type.");
				break;
			}
			
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::AddIndexBuffer(Ref<IndexBuffer> indexBuffer)
	{
		this->Bind();
		m_IndexBuffer = indexBuffer;
		m_IndexBuffer->Bind();
	}
}