#pragma once

#include <memory>

#include "Buffer.h"

namespace GE
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		// Binds Vertex Array
		virtual void Bind() const = 0;

		// Unbinds Vertex Array
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(Ref<VertexBuffer> const& vertexBuffer) = 0;
		virtual void AddIndexBuffer(Ref<IndexBuffer> const& indexBuffer) = 0;

		virtual inline std::vector<Ref<VertexBuffer>> const& GetVertexBuffers() { return m_VertexBuffers; };
		virtual inline Ref<IndexBuffer> const& GetIndexBuffer() { return m_IndexBuffer; };

		static VertexArray* Create();

	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

		uint32_t m_RendererID = 0;
	};
}
