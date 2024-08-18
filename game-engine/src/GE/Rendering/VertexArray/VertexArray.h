#pragma once

#include "Buffers/VertexBuffer.h"
#include "Buffers/IndexBuffer.h"

namespace GE
{
	class VertexArray
	{
	public:
		static Ref<VertexArray> Create();

		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(Ref<VertexBuffer> vertexBuffer) = 0;
		virtual void AddIndexBuffer(Ref<IndexBuffer> indexBuffer) = 0;

		virtual inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() = 0;
		virtual inline const Ref<IndexBuffer> GetIndexBuffer() = 0;
	};
}
