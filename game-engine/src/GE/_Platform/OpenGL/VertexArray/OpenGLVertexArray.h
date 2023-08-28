#pragma once

#include <glad/glad.h>

#include "GE/Rendering/VertexArray/VertexArray.h"

namespace GE
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
	
		virtual inline std::vector<std::shared_ptr<VertexBuffer>> const& GetVertexBuffers() override { return m_VertexBuffers; }
		virtual inline std::shared_ptr<IndexBuffer> const& GetIndexBuffer() override { return m_IndexBuffer; }
	private:
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		uint32_t m_RendererID = 0;
	};
}
