#include "GE/GEpch.h"

#include "OpenGLVertexArray.h"
#include "GE/_Platform/OpenGL/Shader/OpenGLShader.h"

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
		this->Bind();

		vertexBuffer->Bind();

		uint32_t index = 0;
		auto& layout = vertexBuffer->GetLayout();
		for (auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				OpenGLShader::ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)(UINT_PTR)element.Offset);
			index++;
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::AddIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		this->Bind();

		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}