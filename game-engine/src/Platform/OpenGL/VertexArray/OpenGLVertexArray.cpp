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

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		GE_PROFILE_FUNCTION();
		GE_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no Layout.");

		this->Bind();
		vertexBuffer->Bind();

		uint32_t index = 0;
		auto& layout = vertexBuffer->GetLayout();
		for (auto& element : layout)
		{
			switch (element.Type)
			{
			case Shader::ShaderDataType::Float:
			case Shader::ShaderDataType::Float2:
			case Shader::ShaderDataType::Float3:
			case Shader::ShaderDataType::Float4:
			{
				glEnableVertexAttribArray(index);
				glVertexAttribPointer(index,
					element.GetComponentCount(),
					OpenGLShader::ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)(UINT_PTR)element.Offset);
				index++;
				break;
			}
			case Shader::ShaderDataType::Int:
			case Shader::ShaderDataType::Int2:
			case Shader::ShaderDataType::Int3:
			case Shader::ShaderDataType::Int4:
			case Shader::ShaderDataType::Bool:
			{
				glEnableVertexAttribArray(index);
				glVertexAttribIPointer(index,
					element.GetComponentCount(),
					OpenGLShader::ShaderDataTypeToOpenGLBaseType(element.Type),
					layout.GetStride(),
					(const void*)(UINT_PTR)element.Offset);
				index++;
				break;
			}
			case Shader::ShaderDataType::Mat3:
			case Shader::ShaderDataType::Mat4:
			{
				uint8_t count = element.GetComponentCount();
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index,
						count,
						OpenGLShader::ShaderDataTypeToOpenGLBaseType(element.Type),
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

	void OpenGLVertexArray::AddIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		this->Bind();
		m_IndexBuffer = indexBuffer;
		m_IndexBuffer->Bind();
	}
}