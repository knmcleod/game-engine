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

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void AddIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() = 0;
		virtual inline const Ref<IndexBuffer>& GetIndexBuffer() = 0;

		static Ref<VertexArray> Create();

	};
}
