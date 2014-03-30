#pragma once
#include <stdint.h>
#include <vector>
#include "akjIRect.hpp"
#include "akjFontLoader.hpp"
#include "FancyDrawMath.hpp"
#include "akjTextShaper.hpp"
#include "akjColor.hpp"
#include "akjTextureObject.hpp"
#include "akjDoubleVAO.hpp"
#include "akjArrayBuffer.hpp"
#include "akjShaderObject.hpp"
#include "akjWrappingCounter.hpp"
#include <queue>
#include "akjUniformRingBuffer.hpp"
#include "ArrayRef.hpp"
#include "akjLayeredDrawable.hpp"
#include "akj2DGraphicsPrimitives.hpp"
#include "akjBitField.hpp"
#include "Allocator.hpp"
#include "resources/akjStaticResources.hpp"
#include "akjDrawCommander.hpp"

namespace akj
{
	class cFontLoader;
	class cDistanceFieldFont;
	class cHWGraphicsContext;
	enum eTextInfoStyleFlags
	{
		kEmptyFlag = 0,
		kIsVisible = 1,
		kDistanceFieldFont = kIsVisible << 1,
		kTexAtlasFont = kDistanceFieldFont << 1,
		// in the case of errors or broken fonts, we would like to keep the app viable
		kForceHidden = kTexAtlasFont << 1 
	};

	inline cStringRef BuiltinFont(eBuiltinFonts f)
	{
		switch (f)
		{
		case akj::UI_FONT:
			return cStaticResources::get_UI_font();
			break;
		case akj::TEXT_FONT:
			return cStaticResources::get_Text_font();
			break;
		case akj::MONOSPACE_FONT:
			return cStaticResources::get_Mono_font();
			break;
		case akj::NUM_BUILTIN_FONTS:
			break;
		default:
			break;
		}
		AKJ_THROW("whoa, bad enum for builtin font selection");
	}


	// data associated with every string

	
	struct cTextInfo
	{

		cWrappingCounter mDrawOrder;
		int mReferences;
		std::vector<cScreenText::cVertexData> mGlyphs;
		cScreenText::cInstanceData mUniformData;
		uint32_t mID;
		uint32_t mPixelSize;
		cCoord2 mVelocity;
		cCoord2 mAccel;
		iRect mBoundingBox;
		uint32_t mTextureUnit;

		std::string mText;
		float mBaseLine;
		cBitField<eTextInfoStyleFlags> mRequestedFlags;
		cBitField<eTextInfoStyleFlags> mActualFlags;
		eBuiltinFonts mFontType;

		
		void SetVisible(){ 
			if(mRequestedFlags.IsSet(kForceHidden)){
				return;
			}
			mRequestedFlags.Set(kIsVisible);
			mActualFlags.Set(kIsVisible);
		}
		void SetInvisible(){
			mRequestedFlags.Clear(kIsVisible);
			mActualFlags.Clear(kIsVisible);
		}
		bool IsVisible() const {return mActualFlags.IsSet(kIsVisible);}

		void RequestTexAtlasStyle() {
			mRequestedFlags.Clear(kDistanceFieldFont);
			mRequestedFlags.Set(kTexAtlasFont);
		}

		void RequestDistanceFieldStyle() {
			mRequestedFlags.Clear(kTexAtlasFont);
			mRequestedFlags.Set(kDistanceFieldFont);
		}

		void UseDistanceFieldStyle() {
			mActualFlags.Clear(kTexAtlasFont);
			mActualFlags.Set(kDistanceFieldFont);
		}

		void UseTexAtlasStyle() {
			mActualFlags.Clear(kDistanceFieldFont);
			mActualFlags.Set(kTexAtlasFont);
		}


		bool operator<( const cTextInfo& rhs) const
		{
			return mDrawOrder == rhs.mDrawOrder 
				? mTextureUnit < rhs.mTextureUnit 
				: mDrawOrder < rhs.mDrawOrder;
		}
	};

	class cScreenTextFactory: public iLayeredDrawable
	{
		enum {
			kInitialBufferSize = 1 * 1024 * 1024,
			// arbitrary limit, just to catch infinite allocation errors
			kMaxBufferSize = 64 * 1024 * 1024,
			//if there are more than 16 million shapes, we've got issues
			kMaxNumShapes = (16 * 1024 * 1024) - 1,
			kInstanceSize = sizeof(cScreenText::cInstanceData),
			//the limit on bytes is 16384, so 40 is pretty small
			kUniformBlockSize = 80,
			kUniformBlockBytes = kUniformBlockSize*kInstanceSize
			
		};



		struct cSingleFrame
		{
			void Reset()
			{
				mLayers.clear();
				mCommands.Clear();
			}
			void RunCommands(uint32_t layer)
			{
				mCommands.PlayBackSection(layer);
			}
		
			void NextLayer()
			{
				mCommands.NewSection();
			}

			void AddUniformInt(cStringRef name, uint32_t num)
			{
				mCommands.Record<cBindUniformInt>(name, *mShader, num);
			}
			void AddUniformFloat(cStringRef name, float num)
			{
				mCommands.Record<cBindUniformFloat>(name, *mShader, num );
			}
			void AddUniformSwitch(const cBufferRange& range, uint32_t index)
			{
				mCommands.Record<cBindUniformBlock>("StringInstances", range, *mShader, index);
			}
			uRange AddDrawCommand(cArrayBuffer& buffer, uRange range)
			{
				if (range.size == 0) return range;
				mCommands.Record<cDrawPointsCommand>(cBufferRange(buffer, range.start, range.size));
				return range.Next();
			}

			// a 0 sized range implies that the uniform should be incremented
			// with no draw
			std::vector<cSubmittedLayer> mLayers;
			cShaderObject* mShader;
			cDrawCommander mCommands;
		};
		
	public:
		cScreenTextFactory(cHWGraphicsContext& context, cFontLoader& font_loader);
		
		~cScreenTextFactory();
		void MakeSampleLocations(cCoord2 center);
		
		void SetFrameTime(double abs_time);
		void DepthSort();
		void PrepareLayers(std::vector<cSubmittedLayer>& layers);
		virtual void DrawSingleLayer(uint32_t layer);
		virtual uint32_t LayerPriority() const {return kTextPriority;}
		virtual cStringRef LayerName() const {return "text";};
		void PostDrawCleanup();

		void SetFont(tFaceHandle font, eBuiltinFonts font_kind);

		void ChangeEdgeWidth(float incr);
		void ChangeEdgeShift(float incr);
		void ToggleRenderMethod();
		float EdgeWidth(){return mEdgeWidth;}
		float EdgeShift(){return mEdgeShift;}


		cScreenText CreateText(const Twine& text, cCoord2 location,
														uint32_t pixel_size, eBuiltinFonts style = UI_FONT);
		void OnResize();

	private:
		
		template<typename tFunctor> 
		void ForAllGlyphs(uint32_t id, tFunctor func)
		{
			cTextInfo& text = mTextRecords.at(id);
			for(auto & glyph: text.mGlyphs)
			{
				func(glyph);
			}
		}
		
		
		typedef cArray<kUniformBlockSize,cScreenText::cInstanceData> tUniformBlock;

		void MarkAsChanged(uint32_t id)
		{
			if(mTextRecords[id].IsVisible())
			{
				mHasChanges = true;
			}
		}
		void DrawBatches(cShaderObject& shader);
		void UpdateBuffer();
		void AddReference(uint32_t id);
		void RemoveReference(uint32_t id);
		uint32_t ReserveVertices(uint32_t num_verts);
		cTextInfo& AllocateTextInfo();
		void SetVertexAttributes();
		void FreeResources(uint32_t id);

		cScreenText::cInstanceData& UniformData(uint32_t id)
		{
			return UniformData(mTextRecords.at(id));
		}

		cScreenText::cInstanceData& UniformData(const cTextInfo& info)
		{
			return mTextRecords[info.mID].mUniformData;
		}
		void LoadFontTextures(eBuiltinFonts font_kind);
		bool ShapeText(cTextInfo& info, const cCoord2& location);
		void ShapeTextDFF(cDistanceFieldFont& dff, cTextInfo& info,
										const cCoord2& location);
		void ShapeTextTAF(cTexAtlasFont& atlas_font, cTextInfo& info,
										const cCoord2& location);

		void ChangeText(uint32_t id, const Twine& text);
		void SetShaderBindings();
		friend class cScreenText;

		cDoubleVAO mVertexBuffers;
		cGPURingBuffer mUniformBuffer;
		cShaderObject mAlphaShader;

		cHWGraphicsContext& mContext;
		cFontLoader& mFontLoader;

		cArray<NUM_BUILTIN_FONTS,tFaceHandle> mBuiltinFonts;
		cArray<NUM_BUILTIN_FONTS,cTextureObject> mDistanceTextures;
		cArray<NUM_BUILTIN_FONTS,cTextureObject> mAtlasTextures;

		std::priority_queue<uint32_t, std::vector<uint32_t>> mTombstones;

		// list of all the shapes we have record of, sorted by ID only
		std::vector<cTextInfo> mTextRecords;

		// the same, pruned by visibility and sorted by depth
		std::vector<cTextInfo*> mZSortedText;
		
		std::vector<cCoord2> mSampleLocations;

		cScreenText mDebugText;

		cSingleFrame mCurrentFrame;
		
		double mCurrentTime;

		float mEdgeShift;
		float mEdgeWidth;
		float mTopFuzz;
		float mBottomFuzz;
		bool mNeedDepthSort;
		bool mHasChanges;

	};
}
typedef akj::cScreenText::cInstanceData cid_check;
AKJ_SIZE_CHECK(cid_check, 48);
