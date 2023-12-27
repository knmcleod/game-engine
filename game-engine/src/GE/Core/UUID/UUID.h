#pragma once
#include <xhash>

namespace GE
{
	class UUID
	{
	private:
		uint64_t m_UUID;
	public:
		UUID();
		UUID(uint64_t uuid);

		operator uint64_t() const { return m_UUID; }
	};
}

namespace std
{
	template<>
	struct hash<GE::UUID>
	{
		std::size_t operator()(const GE::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}