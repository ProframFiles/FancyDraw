#pragma once
#include "FancyDrawMath.hpp"

// from HarfBuzz
struct hb_font_t;
struct hb_buffer_t;

namespace akj
{
	class FreeTypeFace;
	struct cGlyphPos
	{
		cGlyphPos(uint32_t i, float x, float y)
			:index(i), offset(x, y), padding(0)
		{}
		cCoord2 offset;
		uint32_t index;
		uint32_t padding;
	};

	class cTextShaper
	{
	public:
		cTextShaper(FreeTypeFace& face);

		~cTextShaper();

		const std::vector<cGlyphPos>& 
			ShapeText(cStringRef text);
		std::vector<cGlyphPos>& 
			ShapeText(cStringRef text, std::vector<cGlyphPos>& glyphs);
	private:
		hb_font_t* mFont;
		hb_buffer_t * mBuffer;
		std::vector<cGlyphPos> mGlyphs;
		float mLineHeight;
	};

}
