#pragma once

#include <cstdint>

namespace GE
{
	class UUID
	{
	private:
		uint64_t m_UUID = 0;
	public:
		UUID();
		UUID(uint64_t uuid);

		operator uint64_t() const { return m_UUID; }
	};
}

namespace std
{
	template<typename T> struct hash;

	template<>
	struct hash<GE::UUID>
	{
		size_t operator()(const GE::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};
}