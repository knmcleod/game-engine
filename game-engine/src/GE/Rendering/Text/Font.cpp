#include "GE/GEpch.h"

#include "Font.h"

namespace GE
{

	struct AtlasConfiguration
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		float Scale = 1.0;
			
		uint64_t Seed = 0;
		int32_t ThreadCount = 8;
		bool ExpensiveColoring = false;
	};

	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> func>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, const AtlasConfiguration& atlasConfig,
		std::vector <msdf_atlas::GlyphGeometry> glyphs, msdf_atlas::FontGeometry fontGeometry)
	{
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		msdf_atlas::ImmediateAtlasGenerator<S, N, func, msdf_atlas::BitmapAtlasStorage<T, N>> atlasGenerator(atlasConfig.Width, atlasConfig.Height);
		atlasGenerator.setAttributes(attributes);
		atlasGenerator.setThreadCount(8);
		atlasGenerator.generate(glyphs.data(), glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = atlasGenerator.atlasStorage();
		TextureConfiguration config;
		config.Height = bitmap.height;
		config.Width = bitmap.width;
		config.InternalFormat = ImageFormat::RBG8;
		config.Format = DataFormat::RGB;
		config.GenerateMips = false;

		Buffer dataBuffer((void*)bitmap.pixels, bitmap.height * bitmap.width * (config.InternalFormat == ImageFormat::RBG8 ? 3 : 4));
		Ref<Texture2D> texture = Texture2D::Create(config, dataBuffer);
		return texture;
	}

	Font::Font(const std::filesystem::path& fontPath) : m_MSDFData(new MSDFData())
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
		{
			GE_CORE_ERROR("Failed to load Font Freetype Handle");
			return;
		}

		std::string fontString = fontPath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontString.c_str());
		if (!font)
		{
			GE_CORE_ERROR("Failed to load Font Handle");
			return;
		}

		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		static const CharsetRange charsetRanges[] =
		{
			{ 0x0020, 0x00FF } // Basic Latin + Latin Supplement
		};

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t c = range.Begin; c <= range.End; c++)
				charset.add(c);
		}

		m_MSDFData->FontGeometry = msdf_atlas::FontGeometry(&m_MSDFData->Glyphs);
		int loadedGlyph = m_MSDFData->FontGeometry.loadCharset(font, 1.0, charset);
		
		float scale = 50.0;
		msdf_atlas::TightAtlasPacker atlasPacker;
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setSpacing(0.0);
		atlasPacker.setScale(scale);
		int remaining = atlasPacker.pack(m_MSDFData->Glyphs.data(), m_MSDFData->Glyphs.size());
		
		int width, height;
		atlasPacker.getDimensions(width, height);
		scale = atlasPacker.getScale();
		AtlasConfiguration atlasConfig;
		atlasConfig.Width = width;
		atlasConfig.Height = height;
		atlasConfig.Scale = scale;

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull

		if (atlasConfig.ExpensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = m_MSDFData->Glyphs, &seed = atlasConfig.Seed](int i, int threadNum) -> bool
				{
					unsigned long long glyphSeed = (LCG_MULTIPLIER * (seed ^ i) + LCG_INCREMENT) * !!seed;
					glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
					return true;
				}, m_MSDFData->Glyphs.size()).finish(atlasConfig.ThreadCount);
		}
		else
		{
			unsigned long long glyphSeed = atlasConfig.Seed;
			for (msdf_atlas::GlyphGeometry& glyph : m_MSDFData->Glyphs)
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		m_Texture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Texture", atlasConfig,
			m_MSDFData->Glyphs, m_MSDFData->FontGeometry);

		destroyFont(font);
		deinitializeFreetype(ft);
	}

	Font::~Font()
	{

	}

	// arial default font
	Ref<Font> Font::GetDefault()
	{
		static Ref<Font> DefaultFont;
		if (!DefaultFont)
			DefaultFont = CreateRef<Font>("assets/fonts/arial.ttf");

		return DefaultFont;
	}
}
