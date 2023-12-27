#pragma once
namespace GE
{
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		/*
		*	Sets data on GPU
		*/
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		
		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};
}

