////////////////////////////////////////////////////////////////////////////
//
// file akjFreeTypeFace.hpp
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include "Twine.hpp"
#include "StringRef.hpp"
#include "akjFreeTypeLibrary.hpp"
#include "akjFreeTypeErrors.hpp"
#include "akjLog.hpp"
#include "akjIVec.hpp"
#include "akjAlignedBuffer.hpp"
#include "Bitmap.hpp"
#include "akjTextShaper.hpp"
#include "akjSerialization.hpp"
#include "akjIRect.hpp"




namespace akj
{
class cTextShaper;
class cSerialization;
struct tDeSerializerTrait;




class FreeTypeFace
{
public:
	struct cGlyphIndex
	{
		cGlyphIndex(uint32_t i, uint32_t cc)
			:index(i), CharCode(cc){}
		uint32_t index;
		uint32_t CharCode;
	};
	enum eVersion { kSerializedVersion = 1 };

	FreeTypeFace(FreeTypeLibrary& library, const Twine& p, int pix_size = 18);
	FreeTypeFace(FreeTypeLibrary& library, const FreeTypeFace& other);
	explicit FreeTypeFace(FreeTypeLibrary& library, cStringRef data, 
												int pix_size = 18);


	~FreeTypeFace()
	{
		FT_Done_Face(mFace);
	}

	void GetGlyphSizes(const std::vector<cGlyphIndex>& glyphs,
												std::vector<iRect>& boxes);

	inline FT_Face& ref() {return mFace;};

	std::vector<cGlyphPos>& 
			ShapeText(cStringRef text, std::vector<cGlyphPos>& glyphs)
	{
		return mShaper->ShapeText(text, glyphs);
	}

	float LineHeight() const
	{
		const uint32_t  sz26_6 = mFace->size->metrics.height;
		return static_cast<float>(sz26_6 >> 6)+static_cast<float>(sz26_6&63)/64.0f; 
	}

	const std::vector<cGlyphPos>& ShapeText(cStringRef text)
	{
		return mShaper->ShapeText(text);
	}

	iRect BoundingBox(const std::vector<cGlyphPos>& glyphs);

	inline cStringRef FamilyName() const
	{
		if(mFace->family_name != NULL) { return mFace->family_name; }
		return cStringRef();
	}
	inline cStringRef StyleName() const
	{
		if(mFace->style_name != NULL) { return mFace->style_name; }
		return cStringRef();
	}
	int getSfntNames(std::vector<std::pair<int, std::string> >& stringvec);
	std::string getPSName();

	void setPixelSizes(int w, int h);

	void RenderOutlineInto(	uint32_t index, const cAlignedBitmap& bitmap,
													int x = 0, int y = 0);
	void RenderCharOutlineInto( uint32_t char_code, const cAlignedBitmap& bitmap,
	 int x =0, int y = 0);
	void GetAllASCIIGlyphs(std::vector<cGlyphIndex>& glyphs);

	inline void LoadGlyph(FT_UInt indx)
	{
		uint32_t flags = reinterpret_cast<uint32_t>(mFace->generic.data);
		AKJ_FREETYPE_CHECKED_CALL(FT_Load_Glyph( mFace, indx, flags));
	}

	inline void LoadChar(uint32_t char_code)
	{
		uint32_t flags = reinterpret_cast<uint32_t>(mFace->generic.data);
		AKJ_FREETYPE_CHECKED_CALL(FT_Load_Char( mFace, char_code, flags));
	}

	inline bool IsScalable() const
	{
		return (mFace->face_flags & FT_FACE_FLAG_SCALABLE) > 0;
	}

	inline void renderGlyph(FT_Render_Mode mode = FT_RENDER_MODE_NORMAL)
	{
		AKJ_FREETYPE_CHECKED_CALL(FT_Render_Glyph(mSlot, mode));
	}

	inline uint32_t getCharIndex(FT_ULong  charcode)
	{
		return FT_Get_Char_Index(mFace, charcode);
	}

	cStringRef GlyphName(uint32_t index)
	{
		if(FT_HAS_GLYPH_NAMES(mFace))
		{
			AKJ_FREETYPE_CHECKED_CALL(
				FT_Get_Glyph_Name(mFace,index, mNameBuffer.data(), 
													static_cast<uint32_t>(mNameBuffer.size())));
			return cStringRef(mNameBuffer.ptr<char>());
		}
		return cStringRef();

	}

	void SetLoadFlags(uint32_t flags)
	{
		uint32_t* ptr = reinterpret_cast<uint32_t*>(&mFace->generic.data);
		*ptr = flags;
	}

	void ClearLoadFlag(uint32_t flag)
	{
		uint32_t* ptr = reinterpret_cast<uint32_t*>(&mFace->generic.data);
		*ptr &= ~flag;
	}

	uint32_t FaceHash() const
	{
		return mHashVal;
	}

	const FreeTypeFace& Face() const {return *this;}

	bool FaceEqual(const FreeTypeFace* other) const
	{
		if(other == nullptr) return false;
		return other->FamilyName() == FamilyName() 
						&& other->StyleName() == StyleName();
	}

	inline FT_GlyphSlot& slot() {return mSlot;};

	
	template <class tWriter>
	void Serialize(tWriter& writer) const
	{
		writer.Write(static_cast<unsigned char>('F'));
		writer.Write(static_cast<unsigned char>('T'));
		writer.Write(static_cast<unsigned char>('F'));
		writer.Write(static_cast<uint32_t>(kSerializedVersion));
		auto checksum = writer.StartCheckSum();
		writer.Write(mPixelSize.x);
		writer.Write(mPixelSize.y);
		writer.Write(mFontFileContents);
	}

	static std::unique_ptr<FreeTypeFace> 
		FromData(FreeTypeLibrary& library, cStringRef data);

	template <class tReader>
	static std::unique_ptr<FreeTypeFace>
		FromReader(FreeTypeLibrary& library, tReader& r)
	{
			std::unique_ptr<FreeTypeFace> face;
			face.reset(new FreeTypeFace(library, r, tDeSerializerTrait()));
			return face;
	}

	std::unique_ptr<FreeTypeFace> Duplicate(FreeTypeLibrary& library) const;



private:

	friend class cTextShaper;

	template <class tReader>
	FreeTypeFace(FreeTypeLibrary& library, tReader& reader, tDeSerializerTrait)
		:mShaper()
		,mFace(NULL)
		,mSlot(NULL)
		,mLibrary(&library)
		,mNameBuffer(256)
		,mFileBuffer()
		,mFontFileContents()
	{
		uint32_t version;
		char header[4] = {};
		reader.Read(header[0]);
		reader.Read(header[1]);
		reader.Read(header[2]);
		cStringRef header_string(&header[0]);
		if(header_string != header)
		{
			AKJ_THROW((Twine("Deserialized an invalid Free type face file"
				" with header = \"") +header+"\""));
		}
		reader.Read(version);
		if(version != kSerializedVersion)
		{
			AKJ_THROW((Twine("Deserialized a FreeType face with incorrect version #. "
				"Got ")+Twine(version)+", expected " + Twine(kSerializedVersion)));
		}
		reader.CheckSum();
		reader.Read(mPixelSize.x);
		reader.Read(mPixelSize.y);
		reader.Read(mFontFileContents, mFileBuffer);
		ConstructFromFileData();
		Init();
		AKJ_ASSERT(mLibrary != NULL);
	}

	cStringRef FileIntoBuffer(const Twine& p);
	int ForceUC2Charmap();
	void ConstructFromFileData();
	void Init();



	std::unique_ptr<cTextShaper> mShaper;
	FT_Face mFace;
	FT_GlyphSlot mSlot;
	FreeTypeLibrary* mLibrary;
	AlignedBuffer<16> mFileBuffer;
	AlignedBuffer<16> mNameBuffer;
	cStringRef mFontFileContents;
	uint32_t mHashVal;
	ivec2 mPixelSize;
};

}//end namespace akj

namespace std 
{
	template<> struct hash<const akj::FreeTypeFace*>
	{
		size_t operator()(const akj::FreeTypeFace* face)
		{
			return face->FaceHash();
		}
	};
	template <> struct equal_to<const akj::FreeTypeFace*> 
	{
		bool operator() (const akj::FreeTypeFace* lhs,const akj::FreeTypeFace* rhs)
			const {return lhs->FaceEqual(rhs);}
		typedef const akj::FreeTypeFace* first_argument_type;
		typedef const akj::FreeTypeFace* second_argument_type;
		typedef bool result_type;
	};
}
