#pragma once
#include "GE/Rendering/Textures/Texture.h"
namespace GE
{
	struct MSDFData;

	class Font
	{
	public:
		Font(const std::filesystem::path& fontPath);
		~Font();

		Ref<Texture2D> GetTexture() const { return m_Texture; }
	private:
		Scope<MSDFData> m_MSDFData = nullptr;
		Ref<Texture2D> m_Texture = nullptr;
	};
}

