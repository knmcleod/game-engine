#pragma once

namespace GE
{
	// Currently only supports 32-bit index
	class IndexBuffer
	{
	public:
		static Ref<IndexBuffer> Create(uint32_t size);

		virtual ~IndexBuffer() = default;

		virtual const uint32_t& GetCount() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}