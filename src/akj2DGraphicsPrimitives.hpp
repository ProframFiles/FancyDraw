#pragma once
#include "akjColor.hpp"
#include "FancyDrawMath.hpp"
#include "akj2DVertex.hpp"
#include "akjIVec.hpp"
#include "akjIRect.hpp"
#include "ArrayRef.hpp"
#include "akjWrappingCounter.hpp"
#include <vector>

namespace akj
{
	class c2DPrimitiveFactory;
	class cScreenText;
	struct cTextInfo;
	//////////////////////////////////////////////////////////////////////////
	// Essentially acts as a handle to the primitive, the details of which are
	// held in the factory
	//////////////////////////////////////////////////////////////////////////
	class cRoundedRectangle
	{
	public:
		cRoundedRectangle();
		~cRoundedRectangle();
		cRoundedRectangle Duplicate();
		cRoundedRectangle(cRoundedRectangle&& other);
		cRoundedRectangle& operator =(cRoundedRectangle&& other);
		cRoundedRectangle(const cRoundedRectangle& other);
		cRoundedRectangle& operator =(const cRoundedRectangle& other);
		cRoundedRectangle& TopLeft(cCoord2 pos);
		cCoord2 TopLeft() const;
		cRoundedRectangle& BottomRight(cCoord2 pos);
		cCoord2 BottomRight() const;
		cRoundedRectangle& StrokeWidth(float width);
		cRoundedRectangle& CornerRadius(float radius);
		const cRoundedRectangle& Move(const cCoord2& amount) const;
		cRoundedRectangle& Center(cCoord2 xy);
		cRoundedRectangle& Hide();
		cRoundedRectangle& Show();
		bool IsVisible() const;
		uint32_t ID() const {return mID;}
		const iRect& OccludedRect() const;
		const cRoundedRectangle& AddText(cArrayRef<cScreenText> text) const;
		cRoundedRectangle& StrokeColor(RGBAu8 color);
		cRoundedRectangle& FillColor(RGBAu8 color);
		cRoundedRectangle& StrokeAlpha(uint8_t alpha);
		cRoundedRectangle& FillAlpha(uint8_t alpha);
		cWrappingCounter Depth() const;

		enum { kNumVertices = 16, kNumIndices = 66 };
	private:
		c2DVertex::Instanced& InstanceData() const;
		friend class c2DPrimitiveFactory;
		static void GenerateIndices(uint32_t start_index, 
																std::vector<uint32_t>& vec);

		static void GenerateVertices(std::vector<c2DVertex>& vec);

		void Release()
		{
			mFactory = NULL;
		}
		// the factory is the only thing that can make these
		cRoundedRectangle(c2DPrimitiveFactory* factory, uint32_t mID);	
		

		//////////////////////////////////////////////////////////////////////////
		// Members
		//////////////////////////////////////////////////////////////////////////
		
		c2DPrimitiveFactory* mFactory;
		uint32_t mID;
	};


	class cTexturedQuadFactory;
	struct cQuadInfo;
	class cTexturedQuad
	{
	public:
		cTexturedQuad();
		cTexturedQuad(cTexturedQuad&& other);
		cTexturedQuad(const cTexturedQuad& other);
		~cTexturedQuad();
		cTexturedQuad& operator =(cTexturedQuad&& other);
		cTexturedQuad& operator =(const cTexturedQuad& other);

	private:
		friend class cTexturedQuadFactory;
		cQuadInfo& Info();
		cTexturedQuadFactory* Release();
		cTexturedQuad(cTexturedQuadFactory& factory, uint32_t id);
	
		cTexturedQuadFactory* mParent;
		uint32_t mID;
	};

	enum eBuiltinFonts{
		UI_FONT,
		TEXT_FONT,
		MONOSPACE_FONT,
		NUM_BUILTIN_FONTS
	};

	inline cStringRef ToString(eBuiltinFonts f)
	{
		switch (f)
		{
		case akj::UI_FONT:
			return "UI font";
			break;
		case akj::TEXT_FONT:
			return "Text font";
			break;
		case akj::MONOSPACE_FONT:
			return "Monospace font";
			break;
		case akj::NUM_BUILTIN_FONTS:
			return "";
			break;
		default:
			break;
		}
		return "Error font";
	}

	class cScreenTextFactory;
	class cScreenText
	{
	public:
		cScreenText();
		~cScreenText();
		cScreenText(cScreenText&& other);
		cScreenText& operator =(cScreenText&& other);
		cScreenText& operator =(const cScreenText& other);
		cScreenText(const cScreenText& other);
		const cScreenText& Hide() const;
		const cScreenText& Show() const;

		const cScreenText& FillAlpha(float val) const;
		const cScreenText& Fill(RGBAu8 color) const;

		template<typename tScalar> cScreenText& ChangeTime(tScalar time_change)
		{	return ChangeTimef(static_cast<float>(time_change));	 }
		
		cWrappingCounter Depth() const;
		const cScreenText& Depth(cWrappingCounter depth) const;
		const cCoord2& Pos() const;
		const cScreenText& Move(const cCoord2& shift) const;
		const cScreenText& MoveTo(const cCoord2& pos) const;
		const cScreenText& ChangeText(const Twine& text) const;
		iRect BoundingBox() const;

		struct cVertexData
		{
			cVertexData(const cCoord2& cp, const cCoord2& v, const cCoord2& a,
									const cCoord2& to, const cCoord2& ts,
									float clockwise_radians, uint32_t uniform_index)
				: mGlyphPos(cp)
				, mVelocity(v)
				, mAcceleration(a)
				, mTexPos(to)
				, mTexSize(ts)
				, mClockwiseRadians(clockwise_radians)
				, mIndex(uniform_index)
			{}
			cCoord2 mGlyphPos;
			cCoord2 mVelocity;
			cCoord2 mAcceleration;
			cCoord2 mTexPos;
			cCoord2 mTexSize;
			float mClockwiseRadians;
			uint32_t mIndex;

		};
		struct cInstanceData
		{
			cCoord2 mPosition;
			cCoord2 mMessageSize;
			float mStartTime;
			float mZDepth;
			float mOuterRange;
			float mFuzzFactor;
			RGBAu8 mInnerColor;
			RGBAu8 mSolidColor;
			RGBAu8 mOuterColor;
			uint32_t mFontIndex;
		};
		
	private:
		cScreenText& ChangeTimef(float time_change);
		cTextInfo& Info() const;
		cInstanceData& UniformData() const;
		friend class cScreenTextFactory;
		void Release()
		{
			mFactory = nullptr;
		}
		// the factory is the only thing that can make these
		cScreenText(cScreenTextFactory* factory, uint32_t mID);
		cScreenTextFactory* mFactory;
		uint32_t mID;
	};

}
