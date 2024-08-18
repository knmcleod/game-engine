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
			AudioClip
			// Script(?)
		};

		enum class Status
		{
			None = 0,
			Ready = 1,
			Invalid = 2,
			Loading = 3
		};

		Asset(UUID handle = 0, Type type = Type::None, Status status = Status::None)
			: p_Handle(handle), p_Type(type), p_Status(status) {}
		virtual ~Asset() {}

		/*
		*	Returns Copy of Asset by CreateRef<this>()
		*	By default, returns nullptr.
		*	Implement per inherited Asset
		*/
		virtual inline Ref<Asset> GetCopy() { return nullptr; }
		
		inline const Type GetType() const { return p_Type; }
		inline const UUID& GetHandle() const { return p_Handle; }
		inline const Status& GetStatus() const { return p_Status; }
	protected:
		Type p_Type;
		UUID p_Handle;
		Status p_Status;
	};
}
