#pragma once
#include "Bitmap.hpp"
#include "akjAlignedBuffer.hpp"
#include "akjIVec.hpp"
#include "akjIRect.hpp"
#include "akjFreeTypeFace.hpp"
#include "akjTask.hpp"


namespace akj
{
	
	struct cFixedSizeGlyph
	{
		cFixedSizeGlyph()
		: mIndex(0)
		{}
		cFixedSizeGlyph(int i, int c, const ivec2& pos, 
							ivec2 offset, const ivec2& size)
							: mCharCode(c), mIndex(i)
							, mTexRect({pos.x, pos.y, size.x, size.y})
							, mOffset(offset)
		{}
		bool IsValid() const {return mIndex >= 0;}

		int mCharCode;
		int mIndex;
		iRect mTexRect;
		ivec2 mOffset;
	};

class cTexAtlas
{
	public:
	cTexAtlas(std::vector<cFixedSizeGlyph>&& glyphs,
						cAlignedBitmap bitmap, cAlignedBuffer&& storage,
						uint32_t size)
	: mTexture(bitmap)
	, mGlyphsByCharCode(std::move(glyphs))
	, mStorage(std::move(storage))
	, mFontSize(size)
	{
		mBaseCharCode = mGlyphsByCharCode.front().mCharCode;
		PopulateIndexList();
	}


	const cFixedSizeGlyph& ByIndex(uint32_t i) const
	{
		cFixedSizeGlyph* ptr = nullptr;
		if(	i < mBaseIndex 
				|| i-mBaseIndex >= mGlyphsByIndex.size() 
				|| !(ptr = mGlyphsByIndex.at(i-mBaseIndex)))
		{
			return mInvalidGlyph;
		}
		return *ptr;
	}

	const cFixedSizeGlyph& ByCharCode(uint32_t c) const
	{
		cFixedSizeGlyph* ptr = nullptr;
		if(	c < mBaseCharCode || c-mBaseCharCode >= mGlyphsByIndex.size())
		{
			return mInvalidGlyph;
		}
		return mGlyphsByCharCode.at(c-mBaseCharCode);
	}

	void BlitGlyph(uint32_t char_code, const cAlignedBitmap& dst, int x, int y)
	{
		const cFixedSizeGlyph& glyph = ByCharCode(char_code);
		const auto sub_dst 
			= dst.SubRect<1>(x, y, glyph.mTexRect.width, glyph.mTexRect.height);
		const auto src 
			= mTexture.SubRect<1>( glyph.mTexRect.left, glyph.mTexRect.top,
														glyph.mTexRect.width, glyph.mTexRect.height);
		src.ForEachPixel<uint8_t>(
		[](uint8_t* src, uint8_t* dst_pix){
			*dst_pix = *src;
		}, sub_dst);
	}

	cAlignedBitmap mTexture;
	cAlignedBuffer mStorage;
	std::vector<cFixedSizeGlyph> mGlyphsByCharCode;
		
	const uint32_t mFontSize; //in pix, not points
private:
	cTexAtlas(const cTexAtlas& other)
		:mFontSize(0)	
	{
	}
	void PopulateIndexList();
	std::vector<cFixedSizeGlyph*> mGlyphsByIndex;
	cFixedSizeGlyph mInvalidGlyph;
	uint32_t mBaseIndex;
	uint32_t mBaseCharCode;
};

	
	class cTexAtlasFont
	{
		public:
		enum eVersion { kSerializedVersion = 1 };

		cTexAtlasFont(FreeTypeFace& face,
			std::unique_ptr<FreeTypeLibrary>&& library, 
			std::unique_ptr<cTexAtlas>&& atlas)
		:mFace(*library, face)
		,mAtlases()
		,mNeedTextureRepack(true)
		,mActiveAtlas(nullptr)
		{
			// we want hinting!
			mFace.ClearLoadFlag(FT_LOAD_NO_HINTING);
			uint32_t size = atlas->mFontSize;
			mLibrary = std::move(library);
			AddAtlas(std::move(atlas));
			SetPixelSize(size);
		}

		uint32_t FaceHash() const
		{
			return mFace.FaceHash();
		}

		bool FaceEqual(const cTexAtlasFont* other) const
		{
			return mFace.FaceEqual(&other->mFace);
		}

		const FreeTypeFace& Face() const {return mFace;}

		bool SetPixelSize(uint32_t size);

		const cFixedSizeGlyph& GlyphByIndex(uint32_t index)
		{
			AKJ_ASSERT_AND_THROW(mActiveAtlas);
			const cFixedSizeGlyph& glyph = mActiveAtlas->ByIndex(index);
			if(!glyph.IsValid())
			{
				cStringRef g_name = mFace.GlyphName(index);
				Log::Error("unable to load glyph #%d \"%s\"", index,
					g_name.empty() ? "Unknown" : g_name);
			}
			AKJ_ASSERT(glyph.IsValid());
			return glyph;
		}

		const std::vector<cGlyphPos>& 
			ShapeText(cStringRef text, std::vector<cGlyphPos>& glyphs)
		{
			return mFace.ShapeText(text, glyphs);
		}

		const std::vector<cGlyphPos>& 
			ShapeText(cStringRef text)
		{
			return mFace.ShapeText(text);
		}

		void AddAtlas(std::unique_ptr<cTexAtlas>&& atlas);

		bool HasSize(uint32_t size)
		{
			return GetAtlasForSize(size) != nullptr;
		}

		cAlignedBitmap GetTexture()
		{
			if(mNeedTextureRepack)
			{
				RepackAtlasTexture();
			}
			return mTextureBitmap;
		}

		// pack all the glyph sizes into a single texture
		// will be done lazily when needed otherwise
		void RepackAtlasTexture();

	private:


		cTexAtlas* GetAtlasForSize(uint32_t size)
		{
			auto found = mAtlases.find(size);
			if(found == mAtlases.end() || !found->second)
			{
				return nullptr;
			}
			return found->second.get();
		}
		std::unique_ptr<FreeTypeLibrary> mLibrary;
		FreeTypeFace mFace;
		std::string mFaceName;
		std::unordered_map<uint32_t, std::unique_ptr<cTexAtlas>> mAtlases;
		cAlignedBitmap mTextureBitmap;
		cAlignedBuffer mTextureStorage;
		cTexAtlas* mActiveAtlas;
		bool mNeedTextureRepack;
	};

class cCreateTexAtlasTask : public cTask
{
public:
	cCreateTexAtlasTask(const FreeTypeFace& face, uint32_t pixel_size)
		:cTask("CreateTexAtlas Task")
		, mLibrary(new FreeTypeLibrary)
		, mFontSize(pixel_size)
		, mIsFinished(false)
	{
		mFace = std::move(face.Duplicate(*mLibrary));
		mFace->setPixelSizes(pixel_size, pixel_size);
		//setup in base thread
	}
	~cCreateTexAtlasTask(){};

	std::unique_ptr<cTexAtlas> GetFinishedAtlas();

	virtual void DoWork(uint32_t thread_number = 0)
	{
		InitialPhase();
		RenderGlyphs();
		mIsFinished = true;
	}
	virtual float Progress() const {return mIsFinished ? 1.0f : 0.0f;}
	virtual cStringRef StatusString() const 
	{return mIsFinished ? "Done" : "Processing";}
	virtual bool IsDone() const {return mIsFinished;}

private:

	void InitialPhase()
	{
		std::vector<FreeTypeFace::cGlyphIndex> glyphs;
		glyphs.reserve(100);
		mFace->GetAllASCIIGlyphs(glyphs);
		uint32_t max_char_code = 0;
		uint32_t min_char_code = 0xFFFFFFFF;
		for (const auto& glyph : glyphs)
		{
			if (glyph.CharCode > max_char_code) max_char_code = glyph.CharCode;
			if (glyph.CharCode < min_char_code) min_char_code = glyph.CharCode;
		}
		mGlyphsByCharCode.resize(max_char_code + 1 - min_char_code);
		DeterminePacking(glyphs);
	}
	void DeterminePacking(const std::vector<FreeTypeFace::cGlyphIndex>& glyphs);
	void RenderGlyphs();
	std::unique_ptr<FreeTypeLibrary> mLibrary;
	std::unique_ptr<FreeTypeFace> mFace;
	cAlignedBuffer mStorage;
	cAlignedBitmap mTexBitmap;
	std::vector<cFixedSizeGlyph> mGlyphsByCharCode;
	uint32_t mFontSize;
	bool mIsFinished;
	// members
};



}
namespace std 
{
	template<> struct hash<const akj::cTexAtlasFont*>
	{
		size_t operator()(const akj::cTexAtlasFont* dff)
		{
			return dff->FaceHash();
		}
	};

	template <> struct equal_to<const akj::cTexAtlasFont*> 
	{
		bool operator() (
			const akj::cTexAtlasFont* lhs,
			const akj::cTexAtlasFont* rhs) const 
		{
			return lhs->FaceEqual(rhs);
		}
		typedef const akj::cTexAtlasFont* first_argument_type;
		typedef const akj::cTexAtlasFont* second_argument_type;
		typedef bool result_type;
	};
}
