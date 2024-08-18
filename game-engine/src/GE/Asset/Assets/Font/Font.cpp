#include "GE/GEpch.h"

#include "Font.h"

namespace GE
{
	Font::Font() : Font(UUID())
	{

	}

	Font::Font(UUID handle) : Asset(handle, Asset::Type::Font)
	{
		m_MSDFData = CreateRef<MSDFData>();
	}

	Font::~Font()
	{
		m_MSDFData->Glyphs.clear();
	}

	Ref<Asset> Font::GetCopy()
	{
		GE_CORE_WARN("Could not copy Font Asset. Returning nullptr.");
		return nullptr;
	}
}
