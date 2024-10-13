#pragma once

#include "GE/Core/Core.h"

#include <cstdint>

namespace GE
{
	class Math
	{
	public:
		enum class Type
		{
			None,
			Float, Float2, Float3, Float4,
			Mat3, Mat4,
			Int, Int2, Int3, Int4,
			Bool
		};

		static uint32_t GetTypeSize(Math::Type type)
		{
			switch (type)
			{
			case Math::Type::Float:
				return 4;
			case Math::Type::Float2:
				return 4 * 2;
			case Math::Type::Float3:
				return 4 * 3;
			case Math::Type::Float4:
				return 4 * 4;
			case Math::Type::Mat3:
				return 4 * 3 * 3;
			case Math::Type::Mat4:
				return 4 * 4 * 4;
			case Math::Type::Int:
				return 4;
			case Math::Type::Int2:
				return 4 * 2;
			case Math::Type::Int3:
				return 4 * 3;
			case Math::Type::Int4:
				return 4 * 4;
			case Math::Type::Bool:
				return 1;
			}

			GE_CORE_ASSERT(false, "Unknown Math::Type!");
			return 0;
		}

	};
}
