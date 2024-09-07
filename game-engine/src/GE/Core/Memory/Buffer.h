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
	static inline T GetAligned(T dataSize)
	{
		if (BYTE_ALIGNMENT)
		{

			return dataSize + ((dataSize % BYTE_ALIGNMENT) != 0
								? BYTE_ALIGNMENT - (dataSize % BYTE_ALIGNMENT)
								: 0);
		}
		return dataSize;
	}

	template<typename T>
	static inline size_t GetAlignedOfArray(size_t dataSize)
	{
		// uint64_t : arrayLength
		// dataSize * sizeof(T) : arrayData
		size_t alignedSize = GetAligned(sizeof(uint64_t)) + GetAligned(dataSize * sizeof(T));
		return alignedSize;
	}

	template<typename T>
	static inline size_t GetAlignedOfVec2()
	{
		size_t alignedSize = GetAligned(sizeof(T))		// x position
			+ GetAligned(sizeof(T));					// y position
		return alignedSize;
	}

	template<typename T>
	static inline size_t GetAlignedOfVec3()
	{
		size_t alignedSize = GetAligned(sizeof(T))		// x position
			+ GetAligned(sizeof(T))						// y position
			+ GetAligned(sizeof(T));					// z position
		return alignedSize;
	}

	template<typename T>
	static inline size_t GetAlignedOfVec4()
	{
		size_t alignedSize = GetAligned(sizeof(T))	// x position
			+ GetAligned(sizeof(T))					// y position
			+ GetAligned(sizeof(T))					// z position
			+ GetAligned(sizeof(T));				// w position
		return alignedSize;
	}

	/*
	* Write primitive type(var) to pointer
	*/
	template<class T>
	static inline void WriteAligned(uint8_t*& p, const T& var)
	{
		*(T*)p = var;
		p += GetAligned(sizeof(T));
	}

	/*
	*  Write array size & data to pointer
	*  Example
	*	const char* : STL str.cstr()
	*	const uint8_t* : Buffer.Data
	*/
	template<class T>
	static inline void WriteAlignedArray(uint8_t*& p, const T*& var, size_t size)
	{
		*(size_t*)p = size;
		p += GetAligned(sizeof(size));

		memcpy(p, var, size * sizeof(T));
		p += GetAligned(size * sizeof(T));
	}

	/*
	*
	*/
	template<class T>
	static inline void WriteAlignedVec2(uint8_t*& p, const T& x, const T& y)
	{
		WriteAligned<T>(p, x);
		WriteAligned<T>(p, y);
	}

	/*
	* 
	*/
	template<class T>
	static inline void WriteAlignedVec3(uint8_t*& p, const T& x, const T& y, const T& z)
	{
		WriteAligned<T>(p, x);
		WriteAligned<T>(p, y);
		WriteAligned<T>(p, z);
	}

	/*
	* 
	*/
	template<class T>
	static inline void WriteAlignedVec4(uint8_t*& p, const T& x, const T& y, const T& z, const T& w)
	{
		WriteAligned<T>(p, x);
		WriteAligned<T>(p, y);
		WriteAligned<T>(p, z);
		WriteAligned<T>(p, w);
	}

	/*
	* Reads Aligned primitive data from pointer(p) until end-of-pointer(pEnd) for T type(var)
	*/
	template<typename T>
	static inline bool ReadAligned(const uint8_t*& p, const uint8_t* pEnd, T& var)
	{
		intptr_t signedSize = GetAligned(sizeof(T));

		// Overrun
		if (p + signedSize > pEnd)
			return false;

		var = *(T*)p;
		p += signedSize;

		return true;
	}

	/*
	* Reads Aligned byte array data from pointer(p) until end-of-pointer(pEnd) 
	* for T type(var) of max size(size)
	* Use ReadAligned() for size
	* Uses malloc to allocate var using size * sizeof(T)
	*/
	template<class T>
	static inline bool ReadAlignedArray(const uint8_t*& p, const uint8_t* pEnd, T*& var, const size_t& size)
	{
		// Checks if valid
		if ((intptr_t)size <= 0 ||
			(p + size * sizeof(T)) > pEnd)
			return false;

		memcpy((void*)var, p, size * sizeof(T));
		p += GetAligned(size * sizeof(T));

		return true;
	}

	template<typename T>
	static inline bool ReadAlignedVec2(const uint8_t*& p, const uint8_t* pEnd, T& x, T& y)
	{
		if (ReadAligned<T>(p, pEnd, x) && ReadAligned<T>(p, pEnd, y))
			return true;
		return false;
	}

	template<typename T>
	static inline bool ReadAlignedVec3(const uint8_t*& p, const uint8_t* pEnd, T& x, T& y, T& z)
	{
		if (ReadAligned<T>(p, pEnd, x) && ReadAligned<T>(p, pEnd, y) && ReadAligned<T>(p, pEnd, z))
			return true;
		return false;
	}

	template<typename T>
	static inline bool ReadAlignedVec4(const uint8_t*& p, const uint8_t* pEnd, T& x, T& y, T& z, T& w)
	{
		if (ReadAligned<T>(p, pEnd, x) && ReadAligned<T>(p, pEnd, y) && ReadAligned<T>(p, pEnd, z) && ReadAligned<T>(p, pEnd, w))
			return true;
		return false;
	}

	// Non-owning Raw Buffer
	struct Buffer
	{
	private:
		size_t m_Size = 0;
		uint8_t* m_Data = nullptr;
	
	public:
		operator bool() const { return (bool)m_Data; }
		
		template<typename T>
		T* As() const { return (T*)m_Data; }

		Buffer() = default;
		
		Buffer(size_t size)
		{
			Allocate(size);
		}

		Buffer(const void* data, size_t size)
			: Buffer(size)
		{
			if(data != nullptr && m_Size >= size)
				memcpy(m_Data, data, m_Size);
		}

		void Allocate(size_t size)
		{
			Release();

			m_Data = (uint8_t*)malloc(size * sizeof(uint8_t));
			memset(m_Data, 0, size);
			m_Size = size;
		}

		void Release()
		{
			if (m_Data)
			{
				free((void*)m_Data);
				m_Data = nullptr;
				m_Size = 0;
			}
		}

		const std::size_t& GetSize() const { return m_Size; }

	};
}