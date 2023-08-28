#pragma once

#include <memory>

#include "Buffer.h"

namespace GE
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer> const& vertexBuffer) = 0;
		virtual void AddIndexBuffer(std::shared_ptr<IndexBuffer> const& indexBuffer) = 0;

		virtual inline std::vector<std::shared_ptr<VertexBuffer>> const& GetVertexBuffers() { return m_VertexBuffers; };
		virtual inline std::shared_ptr<IndexBuffer> const& GetIndexBuffer() { return m_IndexBuffer; };

		static VertexArray* Create();

	private:
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		uint32_t m_RendererID = 0;
	};
}
