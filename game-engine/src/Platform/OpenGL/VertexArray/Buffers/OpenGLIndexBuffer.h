#pragma once

#include "GE/Rendering/VertexArray/Buffers/IndexBuffer.h"

namespace GE
{
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t size);
		~OpenGLIndexBuffer() override;

		const uint32_t& GetCount() const { return m_Count; }

		// Binds buffer using OpenGL
		void Bind() const override;
		// Unbinds buffer using OpenGL
		void Unbind() const override;
	private:
		uint32_t m_RendererID;
		uint32_t m_Count = 0;
		uint32_t* m_Indices = nullptr;
	};

}