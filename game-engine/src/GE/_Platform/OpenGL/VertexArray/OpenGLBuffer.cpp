#include "GE/GEpch.h"

#include "OpenGLBuffer.h"

namespace GE
{
	//	-- Vertex Buffer --
	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, float* vertices)
	{
		glCreateBuffers(1, &m_RendererID);
		this->Bind();
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}
	
	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		GE_PROFILE_FUNCTION();
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		GE_PROFILE_FUNCTION();
		glDeleteBuffers(GL_ARRAY_BUFFER, 0);
	}

	//	-- End Vertex Buffer --

	//	-- Index Buffer --
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count, uint32_t* indices)
		: m_Count(count)
	{
		glCreateBuffers(1, &m_RendererID);
		this->Bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		GE_PROFILE_FUNCTION();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		GE_PROFILE_FUNCTION();
		glDeleteBuffers(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	//	-- End Index Buffer --

}