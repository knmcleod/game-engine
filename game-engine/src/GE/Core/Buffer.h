#pragma once
#include <stdint.h>
#include <memory.h>

namespace GE
{
	// Non-owning Raw Buffer
	struct Buffer
	{
		uint8_t* Data = nullptr;
		uint64_t Size = 0;
		
		Buffer() = default;
		
		Buffer(const Buffer&) = default;

		Buffer(uint64_t size)
		{
			Allocate(size);
		}

		Buffer(const void* data, uint64_t size)
			: Data((uint8_t*)data), Size(size)
		{

		}

		static Buffer Copy(Buffer other)
		{
			Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

		void Allocate(uint64_t size)
		{
			Release();

			Data = (uint8_t*)malloc(size);
			Size = size;
		}
		void Release()
		{
			free(Data);
			Data = nullptr;
			Size = 0;
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		operator bool() const
		{
			return (bool)Data;
		}
	};
}