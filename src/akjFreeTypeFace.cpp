////////////////////////////////////////////////////////////////////////////
//
// file FreeTypeFace.cpp
//
////////////////////////////////////////////////////////////////////////////
#include "akjFreeTypeFace.hpp"
#include "MemoryBuffer.hpp"
#include "akjIVec.hpp"
#include "akjTextShaper.hpp"
#include "ttnameid.h"
#include "ConvertUTF.hpp"

#include FT_SFNT_NAMES_H
#include FT_OUTLINE_H
#include FT_BBOX_H
namespace akj
{
	
	struct cFreeTypeBBFinder
	{
		void ExpandForSpan(int y, int count, const FT_Span* spans)
		{
			
			int index = 0;
			while(index < count && spans[index].coverage == 0){ ++index; }
			if(index == count) return;

			const int ymax = static_cast<int>(std::ceil(y + yf));
			const int ymin = static_cast<int>(std::floor(y + yf));
			const int x_left = static_cast<int>
				(std::floor(spans[index].x + xf));

			if(calls == 0)
			{
				bbox.top = ymin;
				bbox.left = x_left;
			}
			else if(bbox.left > x_left)
			{
				bbox.width += bbox.left - x_left;
				bbox.left = x_left;
			}
			++calls;
			//check the left border
			index = count-1;
			while(spans[index].coverage == 0 && index > 0){ --index; }
			const int x_right = static_cast<int>
				(std::ceil(spans[index].x + spans[index].len + xf));
			if(index >= 0 && bbox.Right() <= x_right)
			{
				bbox.width = x_right - bbox.left;
			}

			if (ymin < bbox.top)
			{
				bbox.height += bbox.top - ymin;
				bbox.top = ymin;
			}
			if (ymax >= bbox.Bottom()){
				bbox.height = ymax - bbox.top+1;
			}
		}


		void WriteSpan(int y, int count, const FT_Span* spans)
		{
			uint8_t* ptr;
			int i = 0;
			uint8 cover = 255;
			AKJ_ASSERT(mY-y >=0 && mY-y < static_cast<int>(bitmap.H()));
			while ( i < count)
			{
				ptr = bitmap.PixelData(mX+spans[i].x, mY-y);
				int len = spans[i].len;
				cover = ~static_cast<uint8_t>(spans[i++].coverage);
				while(len--)
				{
					*ptr++ = cover;
				}
			}
		}

		static void WriteSpanStatic(int y, int count,
			const FT_Span* spans, void *user)
		{
			cFreeTypeBBFinder* self = reinterpret_cast<cFreeTypeBBFinder*>(user);
			self->WriteSpan(y, count, spans);
		}
		static void ExpandForSpanStatic(int y, int count, 
															const FT_Span* spans, void *user)
		{
			cFreeTypeBBFinder* self = reinterpret_cast<cFreeTypeBBFinder*>(user);
			self->ExpandForSpan(y, count, spans);
		}
		cFreeTypeBBFinder()
		{Reset();}



		void Reset()
		{
			bbox.left = 0;
			bbox.top = 0;
			bbox.width = 0;
			bbox.height = 0;
			calls=0;
			mX = 0;
			mY = 0;
			xf = 0.0f;
			yf = 0.0f;
		}

		iRect bbox;
		float xf;
		float yf;
		int mX;
		int mY;
		int calls;
		uint32_t mCharCode;
		cAlignedBitmap bitmap;
	};


template <typename tFunctor>
void ForEachGlyph(FT_Face face, tFunctor func)
{
	uint32_t gindex;
	uint32_t charcode = FT_Get_First_Char(face, &gindex);
	while (gindex != 0)
	{
		func(gindex, charcode);
		charcode = FT_Get_Next_Char(face, charcode, &gindex);
	}
}


int FreeTypeFace::getSfntNames(std::vector<std::pair<int, std::string> >& stringVec)
{
	int numStrings = FT_Get_Sfnt_Name_Count(mFace);
	if(numStrings == 0) { return numStrings; }
	stringVec.clear();
	stringVec.reserve(numStrings);

	for(int i = 0; i < numStrings; ++i)
	{
		FT_SfntName fn;
		memset(&fn, 0, sizeof(fn));

		FT_Error ret = FT_Get_Sfnt_Name(mFace, i, &fn);
		if(ret != 0) { AKJ_FREETYPE_ERROR("Sfnt string lookup failed", ret); }
		if(fn.platform_id == 1 && fn.name_id > 0 && fn.name_id < 7)
		{
			stringVec.emplace_back(std::make_pair(
				fn.name_id, 
				std::string((char*)fn.string, fn.string_len)));
		}
	}
	return numStrings;
}
std::string FreeTypeFace::getPSName()
{
	std::string ps;
	int numStrings = FT_Get_Sfnt_Name_Count(mFace);
	uint32_t platforms[] = {TT_PLATFORM_MACINTOSH, TT_PLATFORM_MICROSOFT};
	FT_SfntName fn;
	for(int pi = 0; pi < sizeof(platforms)/sizeof(uint32_t); ++pi)
	{
		for(int i = 0; i < numStrings; ++i)
		{
			memset(&fn, 0, sizeof(fn));
			FT_Error ret = FT_Get_Sfnt_Name(mFace, i, &fn);
			if(ret != 0) { 
				AKJ_FREETYPE_ERROR("Sfnt string lookup failed", ret); 
			}
			if(fn.platform_id == platforms[pi] && fn.name_id == 6)
			{
				ps.assign((char*)fn.string, fn.string_len);
				break;
			}
		}
		if(fn.platform_id == platforms[pi] && fn.name_id == 6)
		{
			break;
		}
	}
	if(fn.platform_id == TT_PLATFORM_MICROSOFT)
	{
		std::string temp = ps;
		ps.clear();
		convertUTF16ToUTF8StringBE(cArrayRef<char>(temp.data(), temp.size()), ps);
	}
	return ps;
}

FreeTypeFace::FreeTypeFace(FreeTypeLibrary& library,const Twine& p, int size)
	: mLibrary(&library)
	, mNameBuffer(256)
	, mFace(NULL)
	, mSlot(NULL)
	, mPixelSize(size, size)
{	
	mFontFileContents = 	FileIntoBuffer(p);
	ConstructFromFileData();
	Init();
}

FreeTypeFace::FreeTypeFace(FreeTypeLibrary& library, cStringRef data, int pix_size /*= 18*/)
	: mLibrary(&library)
	, mNameBuffer(256)
	, mFace(NULL)
	, mSlot(NULL)
	, mPixelSize(pix_size, pix_size)
{
	mFileBuffer.reset(data.size());
	memcpy(mFileBuffer.data(),data.data(), data.size());
	mFontFileContents = cStringRef(mFileBuffer.data(), data.size());
	ConstructFromFileData();
	Init();
}

FreeTypeFace::FreeTypeFace(FreeTypeLibrary& library, const FreeTypeFace& other)
: mLibrary(&library)
, mNameBuffer(256)
, mFace(NULL)
, mSlot(NULL)
, mPixelSize(other.mPixelSize)
{
	cStringRef data = other.mFontFileContents;
	mFileBuffer.reset(data.size());
	memcpy(mFileBuffer.data(),data.data(), data.size());
	mFontFileContents = cStringRef(mFileBuffer.data(), data.size());
	ConstructFromFileData();
	Init();
}

std::unique_ptr<FreeTypeFace> 
FreeTypeFace::Duplicate(FreeTypeLibrary& lib) const
{
	return std::unique_ptr<FreeTypeFace>(
		new FreeTypeFace(lib, mFontFileContents, mPixelSize.x));
}

void FreeTypeFace::ConstructFromFileData()
{
	FT_Error ret = 
		FT_New_Memory_Face(mLibrary->ref(),
												(uchar*)mFontFileContents.data(),
												(FT_Long)mFontFileContents.size(), 0, &mFace);
	if (ret != 0)
	{
		AKJ_FREETYPE_ERROR("Error loading font file ", ret);
	}
}

void FreeTypeFace::Init()
{
	SetLoadFlags(FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);
	ForceUC2Charmap();
	setPixelSizes(mPixelSize.x, mPixelSize.y);
	mHashVal = Hash32(FamilyName()).AddData(StyleName()).AddData(getPSName());
}

std::unique_ptr<FreeTypeFace> 
	FreeTypeFace::FromData(FreeTypeLibrary& library, cStringRef data)
{
	AlignedBuffer<16> temp_buf;
	auto reader = cSerialization::ReaderCompressed(data, temp_buf);
	return FromReader(library, reader);
}

void FreeTypeFace::GetAllASCIIGlyphs(std::vector<cGlyphIndex>& glyphs)
{
	glyphs.clear();
	glyphs.reserve(95);
	ForEachGlyph(mFace, [&](uint32_t gindex, uint32_t charcode){
		if (charcode >= 128 || charcode < 32 ) return;
		glyphs.emplace_back(gindex, charcode);
	});
}

void FreeTypeFace::GetGlyphSizes(const std::vector<cGlyphIndex>& glyphs, 
																		std::vector<iRect>& boxes)
{
	cFreeTypeBBFinder bb_finder;
	FT_Raster_Params rp;
	rp.target = 0;
	rp.flags = FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_AA;
	rp.user = &bb_finder;
	rp.black_spans = 0;
	rp.bit_set = 0;
	rp.bit_test = 0;
	rp.gray_spans = &cFreeTypeBBFinder::ExpandForSpanStatic;
	
	for(const auto& index : glyphs){
		if(index.CharCode == 'T')
		{
			bool d = true;
		}
		LoadGlyph(index.index);
		bb_finder.Reset();
		bb_finder.mCharCode = index.CharCode;
		iRect& bbox = bb_finder.bbox;
		AKJ_FREETYPE_CHECKED_CALL(
			FT_Outline_Render(mLibrary->ref(),&mFace->glyph->outline, &rp));
		boxes.emplace_back(bbox);
	}
}

/*  See http://www.microsoft.com/typography/otspec/name.htm
    for a list of some possible platform-encoding pairs.
    We're interested in 0-3 aka 3-1 - UCS-2.
    Otherwise, fail. If a font has some unicode map, but lacks
    UCS-2 - it is a broken or irrelevant font. What exactly
    Freetype will select on face load (it promises most wide
    unicode, and if that will be slower that UCS-2 - left as
    an excercise to check. */
int  FreeTypeFace::ForceUC2Charmap() {
    for(int i = 0; i < mFace->num_charmaps; i++)
		{
        if ((  (mFace->charmaps[i]->platform_id == 0)
            && (mFace->charmaps[i]->encoding_id == 3))
           || ((mFace->charmaps[i]->platform_id == 3)
            && (mFace->charmaps[i]->encoding_id == 1)))
				{
          AKJ_FREETYPE_CHECKED_CALL(
						FT_Set_Charmap(mFace, mFace->charmaps[i]));
					return 0;
				}
		}
    AKJ_FREETYPE_ERROR("Unable to select UC2 Charmap",0);
}

cStringRef FreeTypeFace::FileIntoBuffer(const Twine& p)
{
	OwningPtr<MemoryBuffer> mapped_file;
	error_code err = MemoryBuffer::getFile(p, mapped_file);
	if (err)
	{
		std::string error_string = ("Failed to open font file \"" + p
			+ "\": " + err.message().c_str()).str();
		throw std::runtime_error(error_string.c_str());
	}
	cStringRef read_file = mapped_file->getBuffer();
	mFileBuffer.reset(read_file.size());
	memcpy(mFileBuffer.data(), read_file.data(), read_file.size());
	return cStringRef(mFileBuffer.data(), read_file.size());
}


void FreeTypeFace::RenderCharOutlineInto(uint32_t char_code, 
							const cAlignedBitmap& bitmap, int x /*= 0*/, int y /*= 0*/)
{
	AKJ_ASSERT(bitmap.Comp() == 1 && bitmap.BPC() == 1);

	LoadChar(char_code);
	cFreeTypeBBFinder bb_finder;
	bb_finder.Reset();
	bb_finder.mX = x;
	bb_finder.mY = y;
	bb_finder.bitmap = bitmap;
	bb_finder.mCharCode = char_code;
	FT_Raster_Params rp;
	rp.target = 0;
	rp.flags = FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_AA;
	rp.user = &bb_finder;
	rp.black_spans = 0;
	rp.bit_set = 0;
	rp.bit_test = 0;
	rp.gray_spans = &cFreeTypeBBFinder::WriteSpanStatic;
	AKJ_FREETYPE_CHECKED_CALL( 
		FT_Outline_Render(mLibrary->ref(),&mFace->glyph->outline, &rp));
}

akj::iRect FreeTypeFace::BoundingBox(const std::vector<cGlyphPos>& glyphs)
{
	cFreeTypeBBFinder bb_finder;
	FT_Raster_Params rp;
	rp.target = 0;
	rp.flags = FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_AA;
	rp.user = &bb_finder;
	rp.black_spans = 0;
	rp.bit_set = 0;
	rp.bit_test = 0;
	rp.gray_spans = &cFreeTypeBBFinder::ExpandForSpanStatic;
	bb_finder.Reset();
	int err = 0;
	for (size_t i = 0; i < glyphs.size() ; ++i)
	{
		bb_finder.xf = glyphs[i].offset.x;
		bb_finder.yf = glyphs[i].offset.y;
		LoadGlyph(glyphs[i].index);
		AKJ_FREETYPE_CHECKED_CALL
			(FT_Outline_Render( mLibrary->ref(),&mFace->glyph->outline, &rp));
	}
	return bb_finder.bbox;
}

void FreeTypeFace::setPixelSizes(int w, int h)
{
	int err = FT_Set_Pixel_Sizes(mFace, w, h);
	mPixelSize.width = w;
	mPixelSize.height = h;
	mShaper.reset(new cTextShaper(*this));
}





}//end namespace akj

