#pragma once

#include "AssetUtils.h"
#include "GE/Core/UUID/UUID.h"

namespace GE
{
	class Asset
	{
	public:
		virtual AssetType GetType() { return AssetType::None; }

		inline const UUID GetHandle() const { return this->handle; }
		inline void SetHandle(UUID handle) { this->handle = handle; }
	protected:
		UUID handle = 0;

		friend class AssetManger;
	};
}
