#pragma once
#include <stdint.h>
#include <memory.h>

namespace GE
{
	/*
	* Based on CPU
	*/
	constexpr auto BYTE_ALIGNMENT = (sizeof(void*));

	/*
	* Returns an aligned size for dataSize
	* if dataSize is divisible by BYTE_ALIGNMENT, add the remaining size
	* else, add 0
	*/
	template<typename T>
	static inline T Aligned(T dataSize)
	{
		if (BYTE_ALIGNMENT)
		{

			return dataSize + ((dataSize % BYTE_ALIGNMENT) != 0
								? BYTE_ALIGNMENT - (dataSize % BYTE_ALIGNMENT)
								: 0);
		}
		return dataSize;
	}

	// Non-owning Raw Buffer
	struct Buffer
	{
		uint64_t Size = 0;
		uint8_t* Data = nullptr;

		operator bool() const { return (bool)Data; }
		
		template<typename T>
		T* As() { return (T*)Data; }

		static Buffer Copy(Buffer other)
		{
			Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

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

	};
}