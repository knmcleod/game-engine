#pragma once

#include "GE/Core/Core.h"
#include "GE/Core/UUID/UUID.h"

namespace GE
{
	class Asset
	{
		friend class AssetManger;
		friend class AssetSerializer;
	public:
		enum class Type
		{
			None = 0,
			Scene,
			Texture2D,
			Font,
			AudioSource
		};

		template<typename T>
		static Ref<T> GetCopy()
		{
			Ref<Asset> asset = GetCopy();
			return std::static_pointer_cast<T>(asset);
		}

		Asset() = default;
		Asset(UUID handle) : p_Handle(handle) {}
		virtual ~Asset() = default;

		virtual inline const Type GetType() const { return Asset::Type::None; }

		inline const UUID& GetHandle() const { return p_Handle; }
	protected:
		virtual Ref<Asset> GetCopy() { return nullptr; }
	protected:
		UUID p_Handle = 0;

	};
}
