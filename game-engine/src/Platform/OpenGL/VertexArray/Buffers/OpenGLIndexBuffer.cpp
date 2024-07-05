#include "GE/GEpch.h"

#include "OpenGLIndexBuffer.h"

#include <glad/glad.h>

namespace GE
{
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count)
		: m_Count(count)
	{
		GE_PROFILE_FUNCTION();
		m_Indices = new uint32_t[count];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < count; i += 6)
		{
			m_Indices[i + 0] = offset + 0;
			m_Indices[i + 1] = offset + 1;
			m_Indices[i + 2] = offset + 2;

			m_Indices[i + 3] = offset + 2;
			m_Indices[i + 4] = offset + 3;
			m_Indices[i + 5] = offset + 0;

			offset += 4;
		}
		glCreateBuffers(1, &m_RendererID);
		this->Bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Count * sizeof(uint32_t), m_Indices, GL_STATIC_DRAW);
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

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}